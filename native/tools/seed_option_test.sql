-- Isolated local muverify fixture. No existing character inventory is modified.
START TRANSACTION;
SELECT ItemCount INTO @serial FROM GameServerInfo WHERE Number=0 FOR UPDATE;
SELECT GameID3 INTO @slot FROM AccountCharacter WHERE Id='muverify' FOR UPDATE;
SET @inventory=CONCAT(
 REPEAT(UNHEX('FF'),14*16),
 UNHEX(CONCAT('00FF28',LPAD(HEX(@serial+1),8,'0'),'7F005000FFFFFFFFFF')),
 REPEAT(UNHEX('FF'),3*16),
 UNHEX(CONCAT('006C28',LPAD(HEX(@serial+2),8,'0'),'49007000FFFFFFFFFF')),
 REPEAT(UNHEX('FF'),25*16),
 UNHEX(CONCAT('004F28',LPAD(HEX(@serial+3),8,'0'),'0300B000FFFFFFFFFF')),
 REPEAT(UNHEX('FF'),191*16));
INSERT INTO `Character` (AccountID,Name,cLevel,Class,Strength,Dexterity,Vitality,Energy,Life,MaxLife,Mana,MaxMana,BP,MaxBP,Shield,MaxShield,Inventory,MagicList,Money,MapNumber,MapPosX,MapPosY,MapDir,DbVersion,Quest,CtlCode)
SELECT AccountID,'MuOptions',cLevel,Class,Strength,Dexterity,Vitality,Energy,Life,MaxLife,Mana,MaxMana,BP,MaxBP,Shield,MaxShield,@inventory,MagicList,Money,0,147,111,MapDir,DbVersion,Quest,0
FROM `Character` WHERE Name='MacGuiDW' AND AccountID='muverify' AND @serial IS NOT NULL AND @slot IS NULL AND NOT EXISTS(SELECT 1 FROM (SELECT Name FROM `Character`) AS existing WHERE Name='MuOptions');
SET @created=ROW_COUNT();
UPDATE AccountCharacter SET GameID3='MuOptions' WHERE Id='muverify' AND GameID3 IS NULL AND @created=1;
UPDATE GameServerInfo SET ItemCount=@serial+3 WHERE Number=0 AND @created=1;
COMMIT;
SELECT @created AS Created,Name,OCTET_LENGTH(Inventory) AS InventoryBytes FROM `Character` WHERE Name='MuOptions' AND AccountID='muverify';
