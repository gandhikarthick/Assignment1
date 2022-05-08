import os
import shutil
# pip install py7zr
import py7zr

files=["include/A1task1.h",
"include/A1task2.h",
"include/task_common.h",
"include/host_timer.h",
"include/initialization.h",
"include/renderdoc.h",
"include/utils_teach.h",
"shaders/vectorAdd_teach.comp",
"shaders/matrixRotNaive_teach.comp",
"shaders/matrixRotOpti_teach.comp",
"src/A1task1_teach.cpp",
"src/A1task2_teach.cpp",
"src/task_common_teach.cpp",
"src/host_timer.cpp",
"src/initialization.cpp",
"src/main.cpp",
"src/renderdoc.cpp",
"src/utils_teach.cpp",
"CMakelists.txt"
]

exercise="Assignment1"
if os.path.isdir(exercise):
    shutil.rmtree(exercise)
os.mkdir(exercise)
os.mkdir(exercise+"/include")
os.mkdir(exercise+"/shaders")
os.mkdir(exercise+"/src")

### Copy all files in their relevant directories ###
for f in files:
    newName=f.replace("_teach","")
    shutil.copyfile(f,exercise+"/"+newName)

shutil.copytree("libs", exercise+"/libs")
with py7zr.SevenZipFile(exercise+".7z", "w") as archive:
    archive.writeall(exercise+"/")

shutil.rmtree(exercise)