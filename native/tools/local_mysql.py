#!/usr/bin/env python3
"""Start the isolated, persistent local MU MySQL container. Never pull/reset data."""

import argparse
import json
import os
from pathlib import Path
import shutil
import socket
import subprocess
import sys
import time


NATIVE = Path(__file__).resolve().parents[1]
RUNTIME = NATIVE / "runtime" / "mysql"
DATA = RUNTIME / "data"
ADMIN = RUNTIME / "secrets" / "admin.cnf"
ENV_FILE = RUNTIME / "docker.env"
INIT = RUNTIME / "init"
NAME = "mu-native-mysql"
IMAGE = "mysql:8.0"
IMAGE_ID = "sha256:b7e118f56c5963e079252e0d6e2978c9c010eb7fc7aaaec100e367796b5dd08f"
LABEL = "com.muonline.native.mysql"
LABEL_VALUE = "ex603-local"
SERVER_OPTIONS = [
    "--mysqlx=0",
    "--character-set-server=latin1",
    "--collation-server=latin1_swedish_ci",
    "--require-secure-transport=ON",
]
# Deliberately select fields: a complete inspect would include secret environment values.
INSPECT_FIELDS = [
    '(index .Config.Labels "' + LABEL + '")',
    ".Config.Image", ".Image", ".Mounts", ".HostConfig.PortBindings",
    ".HostConfig.PublishAllPorts", ".HostConfig.RestartPolicy",
    ".State.Running", ".State.Status", ".Config.Cmd", ".NetworkSettings.Ports",
]
INSPECT_FORMAT = "\n".join("{{json " + field + "}}" for field in INSPECT_FIELDS)


def command(docker, args, timeout=15):
    try:
        return subprocess.run(
            [docker] + args, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
            text=True, timeout=timeout, check=False,
        )
    except subprocess.TimeoutExpired:
        raise RuntimeError("Docker command timed out; no container was removed or reset.") from None
    except OSError:
        raise RuntimeError("Could not execute the Docker CLI.") from None


def docker_cli():
    choices = [shutil.which("docker"), "/usr/local/bin/docker", "/opt/homebrew/bin/docker",
               "/Applications/Docker.app/Contents/Resources/bin/docker"]
    docker = next((path for path in choices if path and os.access(path, os.X_OK)), None)
    if not docker:
        raise RuntimeError("Docker CLI not found. Open Docker Desktop before using this launcher.")
    endpoint = os.environ.get("DOCKER_HOST", "")
    if not endpoint or os.environ.get("DOCKER_CONTEXT"):
        result = command(docker, ["context", "inspect", "--format", "{{.Endpoints.docker.Host}}"])
        if result.returncode:
            raise RuntimeError("Could not identify the current local Docker context.")
        endpoint = result.stdout.strip()
    if not endpoint.startswith("unix://"):
        raise RuntimeError("Select a local Docker Desktop context; remote Docker endpoints are refused.")
    result = command(docker, ["info", "--format", "{{.ServerVersion}}"])
    if result.returncode:
        raise RuntimeError("Docker Desktop is not ready. Open it and retry.")
    return docker


def inspect(docker):
    result = command(docker, ["container", "inspect", "--format", INSPECT_FORMAT, NAME])
    if result.returncode:
        return None
    try:
        values = [json.loads(line) for line in result.stdout.splitlines()]
        if len(values) != len(INSPECT_FIELDS):
            raise ValueError()
        return values
    except (ValueError, TypeError):
        raise RuntimeError("Could not validate the named container; refusing to change it.") from None


def validate(state):
    label, image, image_id, mounts, ports, publish_all, restart, _, status, options, live_ports = state
    expected_ports = {"3306/tcp": [{"HostIp": "127.0.0.1", "HostPort": "3306"}]}
    expected_mounts = {
        "/var/lib/mysql": (DATA, True),
        "/run/secrets/admin.cnf": (ADMIN, False),
        "/docker-entrypoint-initdb.d": (INIT, False),
    }
    correct = (label == LABEL_VALUE and image == IMAGE and image_id == IMAGE_ID
               and ports == expected_ports and publish_all is False
               and restart.get("Name") == "unless-stopped" and options == SERVER_OPTIONS)
    actual_mounts = {entry.get("Destination"): entry for entry in mounts}
    correct = correct and set(actual_mounts) == set(expected_mounts)
    for destination, (source, writable) in expected_mounts.items():
        entry = actual_mounts.get(destination, {})
        correct = correct and (entry.get("Type") == "bind"
                               and entry.get("Source") == str(source)
                               and entry.get("RW") is writable)
    published = {key: value for key, value in (live_ports or {}).items() if value}
    if state[7]:
        correct = correct and published == expected_ports
    if not correct:
        raise RuntimeError("Container mu-native-mysql does not match this launcher's label/image/ports/mounts/options; it was not changed.")
    if status == "paused":
        raise RuntimeError("The matching container is paused; resume it explicitly before retrying.")


def local_paths():
    for path in (RUNTIME.parent, RUNTIME, DATA, RUNTIME / "secrets", ADMIN, ENV_FILE, INIT):
        if path.is_symlink():
            raise RuntimeError("Runtime paths must not be symlinks: " + str(path))
    if not ENV_FILE.is_file() or not ADMIN.is_file() or not INIT.is_dir():
        raise RuntimeError("Prepare private docker.env, secrets/admin.cnf and init/ in " + str(RUNTIME) + " before starting MySQL.")
    if DATA.exists() and not DATA.is_dir():
        raise RuntimeError("The isolated data path is not a directory: " + str(DATA))


def port_available():
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as probe:
            probe.bind(("127.0.0.1", 3306))  # No connect/listen; never displace an existing instance.
    except OSError:
        raise RuntimeError("Local port 127.0.0.1:3306 is already in use or unavailable; no existing instance was stopped.") from None


def query(docker, sql):
    try:
        result = command(docker, ["exec", NAME, "mysql",
            "--defaults-extra-file=/run/secrets/admin.cnf", "--protocol=SOCKET",
            "--batch", "--skip-column-names", "--connect-timeout=2", "-e", sql], timeout=5)
    except RuntimeError:
        return None
    # Never echo raw Docker/mysql errors: they may contain bootstrap SQL or secret values.
    return result.stdout.strip() if result.returncode == 0 else None


def summary(docker):
    result = query(docker, "SELECT VERSION(); SHOW GLOBAL VARIABLES LIKE 'have_ssl';")
    if result is None:
        raise RuntimeError("MySQL status query failed; check the private admin configuration and container state.")
    lines = result.splitlines()
    version = lines[0] if lines else ""
    if not version or len(version) > 80 or any(char not in "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ.-_" for char in version):
        raise RuntimeError("Unexpected MySQL version result; raw output was suppressed.")
    tls = "NOT_REPORTED"
    for line in lines[1:]:
        fields = line.split("\t")
        if len(fields) == 2 and fields[0].lower() == "have_ssl" and fields[1] in ("YES", "NO", "DISABLED"):
            tls = fields[1]
    print("MySQL " + version + " ready at 127.0.0.1:3306; have_ssl=" + tls + ".")
    print("Persistent container: " + NAME + "; data: " + str(DATA))
    print("Database schema/procedure import has not been validated by this launcher.")


def start(docker, state):
    local_paths()
    if state is not None:
        validate(state)
        if state[7]:
            return
        port_available()
        result = command(docker, ["start", NAME], timeout=30)
    else:
        image = command(docker, ["image", "inspect", IMAGE, "--format", "{{.Id}}"])
        if image.returncode or image.stdout.strip() != IMAGE_ID:
            raise RuntimeError("The expected local mysql:8.0 image is unavailable; this launcher will not pull an image.")
        if DATA.exists() and any(DATA.iterdir()):
            raise RuntimeError("Existing data has no matching container identity; refusing ambiguous initialization. Data was preserved: " + str(DATA))
        port_available()
        DATA.mkdir(mode=0o755, parents=True, exist_ok=True)
        result = command(docker, ["run", "--detach", "--name", NAME, "--pull=never",
            "--label", LABEL + "=" + LABEL_VALUE, "--restart", "unless-stopped",
            "--publish", "127.0.0.1:3306:3306", "--env-file", str(ENV_FILE),
            "--mount", "type=bind,src=" + str(DATA) + ",dst=/var/lib/mysql",
            "--mount", "type=bind,src=" + str(ADMIN) + ",dst=/run/secrets/admin.cnf,readonly",
            "--mount", "type=bind,src=" + str(INIT) + ",dst=/docker-entrypoint-initdb.d,readonly",
            IMAGE] + SERVER_OPTIONS, timeout=30)
    if result.returncode:
        raise RuntimeError("Docker could not start mu-native-mysql. No container/data was removed; inspect its state before retrying.")
    current = inspect(docker)
    if current is None:
        raise RuntimeError("The newly started container could not be identified.")
    validate(current)


def main():
    parser = argparse.ArgumentParser(description="Start/check/stop only the isolated local MU MySQL Docker container; never pull or reset data.")
    flags = parser.add_mutually_exclusive_group()
    flags.add_argument("--check-status", action="store_true", help="Read-only admin query against the matching container.")
    flags.add_argument("--stop", action="store_true", help="Explicitly stop the matching persistent container without deleting it/data.")
    args = parser.parse_args()
    docker = docker_cli()
    state = inspect(docker)
    if state is not None:
        validate(state)
    if args.stop or args.check_status:
        if state is None:
            raise RuntimeError("No matching mu-native-mysql container exists.")
        if args.stop:
            result = command(docker, ["stop", "--time", "30", NAME], timeout=45)
            if result.returncode:
                raise RuntimeError("Docker stop failed; no container/data was removed.")
            print("Stopped " + NAME + "; data preserved: " + str(DATA))
        else:
            if not state[7]:
                raise RuntimeError("The matching container is stopped. Run the launcher without flags to start it.")
            if query(docker, "SELECT 1;") != "1":
                raise RuntimeError("MySQL is not ready for authenticated admin queries.")
            summary(docker)
        return 0
    start(docker, state)
    print("Waiting for authenticated MySQL readiness (up to 90 seconds)...", flush=True)
    deadline = time.monotonic() + 90
    while time.monotonic() < deadline:
        current = inspect(docker)
        if current is None:
            raise RuntimeError("The matching container disappeared during startup.")
        validate(current)
        if current[7] and query(docker, "SELECT 1;") == "1":
            summary(docker)
            print("The container keeps running after this launcher exits. Use --stop to stop it.")
            return 0
        time.sleep(1)
    raise RuntimeError("MySQL was not ready within 90 seconds. Container/data were preserved; check --check-status and the private admin configuration.")


if __name__ == "__main__":
    try:
        sys.exit(main())
    except KeyboardInterrupt:
        print("Launcher interrupted. No container was stopped; use --check-status or --stop explicitly.", file=sys.stderr)
        sys.exit(130)
    except (RuntimeError, OSError) as error:
        print("Local MySQL: " + str(error), file=sys.stderr)
        sys.exit(1)
