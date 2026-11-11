-- EX603 character creation and world-entry dependencies.
-- Differences from source: missing class template rolls back; event ITCount reset typo fixed.
DELIMITER $$
CREATE PROCEDURE WZ_CreateCharacter(IN p_account VARCHAR(10),IN p_name VARCHAR(10),IN p_class TINYINT UNSIGNED)
proc: BEGIN
 DECLARE g1,g2,g3,g4,g5 VARCHAR(10);
 DECLARE v_slot INT DEFAULT 0;
 DECLARE EXIT HANDLER FOR SQLEXCEPTION
 BEGIN
  ROLLBACK;
  IF EXISTS(SELECT 1 FROM `Character` WHERE Name=p_name) THEN SELECT 0 AS Result;
  ELSE SELECT 2 AS Result; END IF;
 END;
 IF EXISTS(SELECT 1 FROM `Character` WHERE Name=p_name) THEN SELECT 0 AS Result; LEAVE proc; END IF;
 START TRANSACTION;
 -- Ensure there is a row to lock, serializing concurrent requests on this account.
 INSERT INTO AccountCharacter(Id) VALUES(p_account) ON DUPLICATE KEY UPDATE Id=Id;
 SELECT GameID1,GameID2,GameID3,GameID4,GameID5 INTO g1,g2,g3,g4,g5 FROM AccountCharacter WHERE Id=p_account FOR UPDATE;
 IF g1 IS NULL OR LENGTH(RTRIM(g1))=0 THEN SET v_slot=1;
 ELSEIF g2 IS NULL OR LENGTH(RTRIM(g2))=0 THEN SET v_slot=2;
 ELSEIF g3 IS NULL OR LENGTH(RTRIM(g3))=0 THEN SET v_slot=3;
 ELSEIF g4 IS NULL OR LENGTH(RTRIM(g4))=0 THEN SET v_slot=4;
 ELSEIF g5 IS NULL OR LENGTH(RTRIM(g5))=0 THEN SET v_slot=5;
 ELSE ROLLBACK; SELECT 3 AS Result; LEAVE proc; END IF;
 IF NOT EXISTS(SELECT 1 FROM DefaultClassType WHERE Class=p_class) THEN
  ROLLBACK; SELECT 2 AS Result; LEAVE proc;
 END IF;
 UPDATE AccountCharacter SET
  GameID1=IF(v_slot=1,p_name,GameID1), GameID2=IF(v_slot=2,p_name,GameID2),
  GameID3=IF(v_slot=3,p_name,GameID3), GameID4=IF(v_slot=4,p_name,GameID4),
  GameID5=IF(v_slot=5,p_name,GameID5) WHERE Id=p_account;
 INSERT INTO `Character`(AccountID,Name,cLevel,LevelUpPoint,Class,Strength,Dexterity,Vitality,Energy,Leadership,Inventory,MagicList,Life,MaxLife,Mana,MaxMana,BP,MaxBP,Shield,MaxShield,MapNumber,MapPosX,MapPosY,MDate,LDate,Quest,DbVersion,EffectList)
 SELECT p_account,p_name,Level,LevelUpPoint,p_class,Strength,Dexterity,Vitality,Energy,Leadership,Inventory,MagicList,Life,MaxLife,Mana,MaxMana,0,0,0,0,MapNumber,MapPosX,MapPosY,NOW(),NOW(),Quest,DbVersion,EffectList FROM DefaultClassType WHERE Class=p_class;
 COMMIT;
 SELECT 1 AS Result;
END$$
CREATE PROCEDURE WZ_GetResetInfo(IN p_account VARCHAR(10),IN p_name VARCHAR(10))
BEGIN
 DECLARE v_total INT DEFAULT NULL;
 IF NOT EXISTS(SELECT 1 FROM ResetData WHERE Account=p_account AND Name=p_name) THEN
  INSERT INTO ResetData(Account,Name) VALUES(p_account,p_name);
 END IF;
 SELECT ResetCount INTO v_total FROM `Character` WHERE AccountID=p_account AND Name=p_name;
 UPDATE ResetData SET
  ResetDay=IF(DATEDIFF(CURRENT_DATE,ResetDateDay)>0,0,ResetDay),
  ResetWek=IF(TO_DAYS(CURRENT_DATE)-DAYOFWEEK(CURRENT_DATE)>TO_DAYS(ResetDateWek)-DAYOFWEEK(ResetDateWek),0,ResetWek),
  ResetMon=IF(EXTRACT(YEAR_MONTH FROM CURRENT_DATE)>EXTRACT(YEAR_MONTH FROM ResetDateMon),0,ResetMon)
  WHERE Account=p_account AND Name=p_name;
 SELECT v_total AS Reset,ResetDay,ResetWek,ResetMon FROM ResetData WHERE Account=p_account AND Name=p_name LIMIT 1;
END$$
CREATE PROCEDURE WZ_GetMasterResetInfo(IN p_account VARCHAR(10),IN p_name VARCHAR(10))
BEGIN
 DECLARE v_total INT DEFAULT NULL;
 IF NOT EXISTS(SELECT 1 FROM ResetData WHERE Account=p_account AND Name=p_name) THEN
  INSERT INTO ResetData(Account,Name) VALUES(p_account,p_name);
 END IF;
 SELECT MasterResetCount INTO v_total FROM `Character` WHERE AccountID=p_account AND Name=p_name;
 UPDATE ResetData SET
  MasterResetDay=IF(DATEDIFF(CURRENT_DATE,MasterResetDateDay)>0,0,MasterResetDay),
  MasterResetWek=IF(TO_DAYS(CURRENT_DATE)-DAYOFWEEK(CURRENT_DATE)>TO_DAYS(MasterResetDateWek)-DAYOFWEEK(MasterResetDateWek),0,MasterResetWek),
  MasterResetMon=IF(EXTRACT(YEAR_MONTH FROM CURRENT_DATE)>EXTRACT(YEAR_MONTH FROM MasterResetDateMon),0,MasterResetMon)
  WHERE Account=p_account AND Name=p_name;
 SELECT v_total AS MasterReset,MasterResetDay,MasterResetWek,MasterResetMon FROM ResetData WHERE Account=p_account AND Name=p_name LIMIT 1;
END$$
CREATE PROCEDURE WZ_GetEventEntryInfo(IN p_account VARCHAR(10),IN p_name VARCHAR(10))
BEGIN
 IF NOT EXISTS(SELECT 1 FROM EventEntryCount WHERE Account=p_account AND Name=p_name) THEN
  INSERT INTO EventEntryCount(Account,Name) VALUES(p_account,p_name);
 END IF;
 UPDATE EventEntryCount SET BCCount=0,CCCount=0,DSCount=0,DGCount=0,ITCount=0,IGCount=0,LastDate=NOW()
  WHERE Account=p_account AND Name=p_name AND DATEDIFF(CURRENT_DATE,LastDate)>0;
 SELECT BCCount,CCCount,DSCount,DGCount,ITCount,IGCount FROM EventEntryCount WHERE Account=p_account AND Name=p_name LIMIT 1;
END$$
CREATE PROCEDURE WZ_GetItemSerial()
proc: BEGIN
 DECLARE v_serial INT DEFAULT NULL;
 DECLARE EXIT HANDLER FOR SQLEXCEPTION BEGIN ROLLBACK; SELECT -1 AS ItemSerial; END;
 START TRANSACTION;
 -- This EX603 dump contains one counter row. Refuse an ambiguous multi-counter deployment.
 IF (SELECT COUNT(*) FROM GameServerInfo)<>1 THEN ROLLBACK; SELECT -1 AS ItemSerial; LEAVE proc; END IF;
 UPDATE GameServerInfo SET ItemCount=ItemCount+1;
 SELECT ItemCount INTO v_serial FROM GameServerInfo LIMIT 1;
 COMMIT;
 SELECT v_serial AS ItemSerial;
END$$
DELIMITER ;
