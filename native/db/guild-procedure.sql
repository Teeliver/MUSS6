-- MySQL implementation of the MSSQL WZ_GuildCreate procedure used by
-- DataServer/GuildManager.cpp::AddGuild. Returns 0 on success, 1 when the
-- guild name exists, 2 when the master is already in a guild.
DROP PROCEDURE IF EXISTS `WZ_GuildCreate`;

DELIMITER $$
CREATE PROCEDURE `WZ_GuildCreate`(IN p_guild VARCHAR(8), IN p_master VARCHAR(10))
BEGIN
  DECLARE v_guild INT DEFAULT 0;
  DECLARE v_member INT DEFAULT 0;

  SELECT COUNT(*) INTO v_guild FROM `Guild` WHERE `G_Name` = p_guild;
  SELECT COUNT(*) INTO v_member FROM `GuildMember` WHERE `Name` = p_master;

  IF v_guild > 0 THEN
    SELECT 1;
  ELSEIF v_member > 0 THEN
    SELECT 2;
  ELSE
    INSERT INTO `Guild`
      (`G_Name`,`G_Mark`,`G_Score`,`G_Master`,`G_Count`,`G_Notice`,`G_Type`,`G_Rival`,`G_Union`,`MemberCount`)
    VALUES
      (p_guild,UNHEX(REPEAT('00',32)),0,p_master,1,'',0,0,0,1);
    INSERT INTO `GuildMember` (`Name`,`G_Name`,`G_Level`,`G_Status`)
    VALUES (p_master,p_guild,1,128);
    SELECT 0;
  END IF;
END$$
DELIMITER ;
