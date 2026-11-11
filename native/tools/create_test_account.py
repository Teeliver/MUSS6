#!/usr/bin/env python3
"""Create an isolated test account without printing or passing its password in argv."""
from pathlib import Path
import secrets, string, subprocess, os
root=Path(__file__).resolve().parents[1]
file=root/'runtime/client/credentials.txt'
if file.exists():
    print('Existing private test credentials retained.'); raise SystemExit(0)
file.parent.mkdir(parents=True,exist_ok=True)
password=''.join(secrets.choice(string.ascii_letters+string.digits) for _ in range(10))
account='muverify'
hexpass=password.encode().hex()
sql=f"INSERT INTO MEMB_INFO(memb___id,memb__pwd,memb_name,sno__numb,bloc_code,ctl1_code) VALUES ('{account}',CONVERT(0x{hexpass} USING latin1),'Verify','000000000000000000','0','0');"
result=subprocess.run(['docker','exec','-i','mu-native-mysql','mysql','--defaults-extra-file=/run/secrets/admin.cnf','MuOnline'],input=sql,text=True,capture_output=True)
if result.returncode: raise SystemExit('Test account creation failed; no credential file was written. Existing accounts were not overwritten.')
fd=os.open(file,os.O_WRONLY|os.O_CREAT|os.O_EXCL,0o600)
with os.fdopen(fd,'w') as stream:stream.write(account+'\n'+password+'\n')
print('Created test account muverify. Private credentials: '+str(file))
