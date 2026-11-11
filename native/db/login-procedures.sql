-- EX603 account service procedures. Import into the selected MuOnline database.
-- Schema migration must precede this file. No account passwords are included.
DELIMITER $$
CREATE PROCEDURE WZ_GetAccountLevel(IN p_account VARCHAR(10))
BEGIN
  DECLARE v_level INT DEFAULT NULL;
  DECLARE v_expiry DATETIME DEFAULT NULL;
  SELECT AccountLevel, AccountExpireDate INTO v_level, v_expiry
    FROM MEMB_INFO WHERE memb___id=p_account LIMIT 1;
  IF v_level<>0 AND NOW()>v_expiry THEN
    SET v_level=0;
    UPDATE MEMB_INFO SET AccountLevel=0 WHERE memb___id=p_account;
  END IF;
  SELECT v_level AS AccountLevel, v_expiry AS AccountExpireDate;
END$$
CREATE PROCEDURE WZ_SetAccountLevel(IN p_account VARCHAR(10), IN p_level INT, IN p_seconds INT)
BEGIN
  DECLARE v_level INT DEFAULT NULL;
  DECLARE v_expiry DATETIME DEFAULT NULL;
  SELECT AccountLevel, AccountExpireDate INTO v_level, v_expiry
    FROM MEMB_INFO WHERE memb___id=p_account LIMIT 1;
  IF v_level=p_level THEN
    SET v_expiry=DATE_ADD(v_expiry, INTERVAL p_seconds SECOND);
  ELSE
    SET v_expiry=DATE_ADD(NOW(), INTERVAL p_seconds SECOND);
  END IF;
  UPDATE MEMB_INFO SET AccountLevel=p_level, AccountExpireDate=v_expiry
    WHERE memb___id=p_account;
END$$
CREATE PROCEDURE WZ_CONNECT_MEMB(IN p_account VARCHAR(10), IN p_server VARCHAR(50), IN p_ip VARCHAR(20))
BEGIN
  IF EXISTS(SELECT 1 FROM MEMB_STAT s INNER JOIN MEMB_INFO i
    ON s.memb___id=i.memb___id WHERE i.memb___id=p_account) THEN
    UPDATE MEMB_STAT SET ConnectStat=1, ServerName=p_server, IP=p_ip,
      ConnectTM=NOW() WHERE memb___id=p_account;
  ELSE
    INSERT INTO MEMB_STAT(memb___id,ConnectStat,ServerName,IP,ConnectTM)
      VALUES(p_account,1,p_server,p_ip,NOW());
  END IF;
END$$
CREATE PROCEDURE WZ_DISCONNECT_MEMB(IN p_account VARCHAR(10))
BEGIN
  IF EXISTS(SELECT 1 FROM MEMB_STAT s INNER JOIN MEMB_INFO i
    ON s.memb___id=i.memb___id WHERE i.memb___id=p_account) THEN
    UPDATE MEMB_STAT SET ConnectStat=0, DisconnectTM=NOW()
      WHERE memb___id=p_account;
  END IF;
END$$
DELIMITER ;
