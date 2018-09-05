print("Studio Data:",json.encode(Studio.DATA))
print("Studio Connected:",Studio.CONNECTED)

print("Project File:",Studio.DATA.projectFile)
print("Project Dir:",Studio.DATA.projectDir)

-- List project files
print(json.encode(Studio.listFiles()))
-- Create a folder named 'TEST'
print(json.encode(Studio.addFolder(nil,"TEST")))
-- Create a file 'sound.sfx' inside the TEST folder
print(json.encode(Studio.addFile({ "TEST" },"sound.sfx",{ excludeFromExecution=true })))
