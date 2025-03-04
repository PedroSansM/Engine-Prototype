import os
import shutil
import multiprocessing



print("Select the build type:")
print("\t0 - Debug")
print("\t1 - Release")
option = int(input("==> "))
buildType = ""
if option == 0:
    buildType = "Debug"
else:
    buildType = "Release"
print("Making a "+buildType+" build...")
os.system("cmake -S . -B build/"+buildType+" -D MAIN_TARGET=DommusEditor -D PROJECT_PATH_DEFINED=OFF -D CMAKE_BUILD_TYPE="+buildType+" -D CMAKE_EXPORT_COMPILE_COMMANDS=1 -D CMAKE_RUNTIME_OUTPUT_DIRECTORY=bin -D GLFW_BUILD_WAYLAND=OFF -D GLFW_BUILD_X11=ON -G \"Unix Makefiles\"")
try:
    numberOfCores = multiprocessing.cpu_count();
    os.system("cmake --build build/"+buildType+" -j "+str(numberOfCores))
except:
    print("Fail to get the number of CPU cores. Omitting the jobs flag!")
    os.system("cmake --build build/"+buildType)
if os.path.exists("build/compile_commands.json"):
    os.remove("build/compile_commands.json")
shutil.move("build/"+buildType+"/compile_commands.json", "build")
print(buildType+" build completed!")
