-- EX603 DataServer calls this with account/name after GameServer policy checks.
-- Return 0 for a guild member/missing owner, 3 for blocked characters, 1 on success.
-- Clear slots atomically; the original DataServer's subsequent slot cleanup is harmless.
DELIMITER $$
CREATE PROCEDURE WZ_DeleteCharacter(IN p_account VARCHAR(10),IN p_name VARCHAR(10))
proc: BEGIN
 DECLARE v_control INT DEFAULT NULL;
 DECLARE v_account VARCHAR(10);
 DECLARE EXIT HANDLER FOR SQLEXCEPTION BEGIN ROLLBACK; RESIGNAL; END;
 START TRANSACTION;
 SELECT Id INTO v_account FROM AccountCharacter WHERE Id=p_account FOR UPDATE;
 SELECT CtlCode INTO v_control FROM `Character` WHERE AccountID=p_account AND Name=p_name FOR UPDATE;
 IF v_control IS NULL OR EXISTS(SELECT 1 FROM GuildMember WHERE Name=p_name) THEN
  ROLLBACK; SELECT 0 AS Result; LEAVE proc;
 END IF;
 IF (v_control & 18)<>0 THEN ROLLBACK; SELECT 3 AS Result; LEAVE proc; END IF;
 DELETE FROM `Character` WHERE AccountID=p_account AND Name=p_name;
 UPDATE AccountCharacter SET
 GameID1=IF(GameID1=p_name,NULL,GameID1),GameID2=IF(GameID2=p_name,NULL,GameID2),
 GameID3=IF(GameID3=p_name,NULL,GameID3),GameID4=IF(GameID4=p_name,NULL,GameID4),
 GameID5=IF(GameID5=p_name,NULL,GameID5) WHERE Id=p_account;
 COMMIT;
 SELECT 1 AS Result;
END$$
DELIMITER ;
