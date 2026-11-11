#!/usr/bin/env python3
"""Explicit integration checks against MuOnlineNativeValidation2 only; never production."""
import subprocess, secrets
from concurrent.futures import ThreadPoolExecutor
DB='MuOnlineNativeValidation2'
def query(sql):
 r=subprocess.run(['docker','exec','-i','mu-native-mysql','mysql','--defaults-extra-file=/run/secrets/admin.cnf','--batch','--skip-column-names',DB],input=sql,text=True,capture_output=True)
 if r.returncode:raise RuntimeError('Validation SQL failed (diagnostic output withheld).')
 return [line.split('\t') for line in r.stdout.splitlines()]
def check(ok,message):
 if not ok:raise RuntimeError(message)
suffix=secrets.token_hex(3)
account='A'+suffix
names=['N'+str(i)+suffix for i in range(5)]
for name in names:check(query(f"CALL WZ_CreateCharacter('{account}','{name}',0);")==[['1']],'Create failed')
check(query(f"CALL WZ_CreateCharacter('{account}','X{suffix}',0);")==[['3']],'Full-slot result failed')
check(query(f"CALL WZ_CreateCharacter('{account}','{names[0]}',0);")==[['0']],'Duplicate result failed')
check(query(f"SELECT COUNT(*) FROM `Character` WHERE AccountID='{account}';")==[['5']],'Wrong character count')
invalid='I'+suffix
check(query(f"CALL WZ_CreateCharacter('{invalid}','Bad{suffix}',7);")==[['2']],'Invalid-template result failed')
check(query(f"SELECT COUNT(*) FROM AccountCharacter WHERE Id='{invalid}';")==[['0']],'Rollback left an orphan account slot')
check(query(f"SELECT OCTET_LENGTH(Inventory),OCTET_LENGTH(MagicList),OCTET_LENGTH(Quest),OCTET_LENGTH(EffectList) FROM `Character` WHERE Name='{names[0]}';")==[['3776','180','50','208']],'Binary template lengths changed')
check(query(f"CALL WZ_GetResetInfo('{account}','{names[0]}');")==[['0','0','0','0']],'Reset defaults failed')
check(query(f"CALL WZ_GetMasterResetInfo('{account}','{names[0]}');")==[['0','0','0','0']],'Master reset defaults failed')
check(query(f"CALL WZ_GetEventEntryInfo('{account}','{names[0]}');")==[['0']*6],'Event defaults failed')
query(f"UPDATE EventEntryCount SET ITCount=5,LastDate=DATE_SUB(CURRENT_DATE,INTERVAL 1 DAY) WHERE Account='{account}' AND Name='{names[0]}';")
check(query(f"CALL WZ_GetEventEntryInfo('{account}','{names[0]}');")==[['0']*6],'Daily event reset failed')
check(query(f"SELECT ITCount FROM EventEntryCount WHERE Account='{account}' AND Name='{names[0]}';")==[['0']],'IT reset was not persisted')
check(query(f"SELECT COUNT(*) FROM ResetData WHERE Account='{account}' AND Name='{names[0]}';")==[['1']],'Reset reads inserted duplicate rows')
check(query(f"SELECT COUNT(*) FROM EventEntryCount WHERE Account='{account}' AND Name='{names[0]}';")==[['1']],'Event reads inserted duplicate rows')
collision='C'+suffix
with ThreadPoolExecutor(max_workers=2) as pool:
 results=list(pool.map(lambda owner:query(f"CALL WZ_CreateCharacter('{owner}','{collision}',0);"),['B'+suffix,'D'+suffix]))
check(sorted(results)==[[['0']],[['1']]],'Concurrent name collision did not produce one winner')
check(query(f"SELECT COUNT(*) FROM `Character` WHERE Name='{collision}';")==[['1']],'Concurrent duplicate row')
old=query('SELECT ItemCount FROM GameServerInfo;')[0][0]
try:
 query('UPDATE GameServerInfo SET ItemCount=2147483647;')
 check(query('CALL WZ_GetItemSerial();')==[['-1']],'Overflow result failed')
 check(query('SELECT ItemCount FROM GameServerInfo;')==[['2147483647']],'Overflow was not rolled back')
finally:query('UPDATE GameServerInfo SET ItemCount='+old+';')
print('PASS: create, duplicate, five-slot limit, missing-template rollback, real binary templates, reset/event reads, daily IT reset, concurrent name collision, serial overflow rollback.')
# delete-character-procedure.sql must be installed in this isolated validation DB.
delete_name='D'+suffix
check(query(f"CALL WZ_CreateCharacter('{account}','{delete_name}',32);")==[['3']],'Full account unexpectedly created Elf')
probe_account='D'+suffix
check(query(f"CALL WZ_CreateCharacter('{probe_account}','{delete_name}',32);")==[['1']],'Delete probe creation failed')
check(query(f"CALL WZ_DeleteCharacter('{account}','{delete_name}');")==[['0']],'Wrong owner deletion accepted')
check(query(f"SELECT COUNT(*) FROM `Character` WHERE Name='{delete_name}';")==[['1']],'Wrong owner removed character')
check(query(f"CALL WZ_DeleteCharacter('{probe_account}','{delete_name}');")==[['1']],'Owned character deletion failed')
check(query(f"SELECT COUNT(*) FROM `Character` WHERE Name='{delete_name}';")==[['0']],'Deleted character persisted')
check(query(f"SELECT COUNT(*) FROM AccountCharacter WHERE Id='{probe_account}' AND ('{delete_name}' IN (GameID1,GameID2,GameID3,GameID4,GameID5));")==[['0']],'Deleted slot persisted')
query(f"DELETE FROM AccountCharacter WHERE Id='{probe_account}';")
print('PASS: wrong-owner deletion rejected; owned Elf removed from Character and account slots.')
