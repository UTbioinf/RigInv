#!/usr/bin/env python

import subprocess
import errno
import os
import shutil
import argparse
import distutils.spawn
import logging

config = {
        "project": "riginv", # required
        "version": "1.1", # basic version, required
        "start_hex": "f1cfe30a29b52fcc60cf21fa0cd83f3bd3e0e0ce", # required
        "python": # optional
            { 
                "bin": [("riginv.py", "riginv")],
                "lib": ["__init__.py", "util.py", "ManyFiles.py"],
                "libexec_bin": [("bamExtractor.py", )],
                "dependencies": "requirements.txt",
            },
        "cpp": # optional 
            {
                "detect_brew": True,
                "parameters": [("LOGGER_LEVEL", "3")]
            } 
        # "makefile": ["file1.mk", "file2.mk"]
    }

"""
The structure of source and destination

source

/
|- install.py
|- build/  # will be created automatically by this script
|- other files and directories
|- src/
    |- cpp/
        |- CMakeLists.txt  # installation is determined by this script
        |- cpp source files, and maybe python packages
    |- makefiles/
        |- all makefiles
    |- python/
        |- requirements.txt
        |- lib/
            |- all files of libraries used in this project
        |- src/
            |- all executable files

destination
/
|- bin/
    |- all executables, including python and cpp
    |- version.txt
|- lib/
    |- cpp libraries
    |- python/
        |- <project>_lib
|- include/
    |- cpp headers
|- libexec/
    |- bin/
        |- aux executables, including python and cpp
    |- makefiles/
        |- all makefiles

"""

def mkdir_p(path):
    try:
        os.makedirs(path)
    except OSError as err:
        if err.errno != errno.EEXIST:
            raise

def get_version(args):
    if os.path.isdir( os.path.join(args.source_dir, ".git") ):
        return "{}.{}".format(config["version"], subprocess.check_output(["git", "rev-list", "--count", "{}..HEAD".format(config["start_hex"])]).rstrip())
    else:
        return config["version"]

def install_version(args):
    with open(os.path.join(args.install["bin"], "version.txt"), "w") as fout:
        this_version = get_version(args)
        args.logger.info("Install version {}".format( this_version ))
        fout.write(this_version)

def install_python(args):
    py = config.get("python")
    if not py:  return
    # install dependencies
    pyitem = py.get("dependencies")
    if pyitem:
        args.logger.info("Install python dependencies")
        subprocess.check_call(["pip", "install", "-r", os.path.join(args.source["python_root"], pyitem)])

    # install python bin
    pyitem = py.get("bin")
    if pyitem:
        args.logger.info("Install python bin")
        mkdir_p( args.install["bin"] )
        for each in pyitem:
            if 1 <= len(each) <= 2:
                target_file = os.path.join(args.install["bin"], each[-1])
                shutil.copyfile(os.path.join(args.source["python"], each[0]), target_file)
                subprocess.check_call(["chmod", "+x", target_file])
            else:
                raise ValueError("Unsupported tuple length: {}".format(str(each)))

    # install lib
    pyitem = py.get("lib")
    if pyitem:
        args.logger.info("Install python lib")
        mkdir_p( args.install["libpython"] )
        for each in pyitem:
            shutil.copyfile(os.path.join(args.source["pylib"], each), os.path.join(args.install["libpython"], each))
    
    # install libexec_bin
    pyitem = py.get("libexec_bin")
    if pyitem:
        args.logger.info("Install python libexec/bin")
        mkdir_p( args.install["libexec_bin"] )
        for each in pyitem:
            if 1 <= len(each) <= 2:
                target_file = os.path.join(args.install["libexec_bin"], each[-1])
                shutil.copyfile(os.path.join( args.source["python"], each[0]), target_file)
                subprocess.check_call(["chmod", "+x", target_file])
            else:
                raise ValueError("Unsupported tuple length: {}".format(str(each)))

def install_cpp(args):
    cpp = config.get("cpp")
    if cpp == None: return
    
    args.logger.info("Install cpp")
    mkdir_p( args.build_dir )
    if cpp.get("detect_brew"):
        if distutils.spawn.find_executable("brew") != None:
            args.brew = subprocess.check_output(["brew", "--prefix"]).strip()

    cmd_args = ["cmake", args.source["cpp"], "-DCMAKE_INSTALL_PREFIX={}".format(args.install_dir)]
    if args.build_debug:
        cmd_args.append("-DCMAKE_BUILD_TYPE=Debug")
    else:
        cmd_args.append("-DCMAKE_BUILD_TYPE=Release")

    cpp_parameters = cpp.get("parameters", [])
    for each in cpp_parameters:
        if len(each) == 1: # optional parameter
            value = getattr(args, each[0])
            if value != None:
                cmd_args.append("-D{}".format(each[0]))
        elif len(each) == 2:
            value = getattr(args, each[0])
            if value != None:
                cmd_args.append("-D{}={}".format(each[0], value))
            elif each[1] == False: # this one is optional
                pass
            elif each[1] != None: # use the default value
                cmd_args.append("-D{}={}".format(each[0], each[1]))
            else:
                raise ValueError("CPP parameter is not set for {}".format(str(each)))
        elif len(each) == 3:
            value = getattr(args, each[0])
            if value != None:
                cmd_args.append("-D{}={}".format(each[0], value))
            elif each[1] == "brew" and hasattr(args, "brew"):
                cmd_args.append("-D{}={}".format(each[0], args.brew))
            elif each[2] == False: # this one is optional
                pass
            elif each[2] != None: # use the default value
                cmd_args.append("-D{}={}".format(each[0], each[2]))
            else:
                raise ValueError("CPP parameter is not set for {}".format(str(each)))
        else:
            raise ValueError("CPP parameter {} has illegal number of options".format(each))
    os.chdir( args.build_dir )
    args.logger.info("run cmake: {}".format(str(cmd_args)))
    subprocess.check_call(cmd_args) # call cmake
    subprocess.check_call(["make", "install"])

def install_makefile(args):
    makefile = config.get("makefile")
    if not makefile:    return
    args.logger.info("Install makefiles")
    mkdir_p( args.install["libexec_mk"] )
    for each in makefile:
        shutil.copyfile(os.path.join(args.source["makefile"], each), os.path.join(args.install["libexec_mk"], each))
        
            
            

def parse_args():
    parser = argparse.ArgumentParser(description = "Installing script")
    parser.add_argument("--prefix", help="Installing prefix (default: <repo dir>/build/local)")
    cpp = config.get("cpp")
    if cpp != None:
        parser.add_argument("--build-debug", action="store_true", help="Set CPP build type as DEBUG install of release")
        cpp_parameters = cpp.get("parameters", [])
        for each in cpp_parameters:
            if len(each) == 1:
                parser.add_argument("--{}".format(each[0]), action="store_true", help="CPP parameter")
            else:
                parser.add_argument("--{}".format(each[0]), help="CPP parameter")
    parser.add_argument("-v", "--verbose",  action="store_true", help="Verbose")
    return parser.parse_args()

def main():
    args = parse_args()
    if args.verbose:
        logging.basicConfig(format="[%(asctime)s] [%(levelname)s]: %(message)s", level=logging.INFO)
    else:
        logging.basicConfig(format="[%(asctime)s] [%(levelname)s]: %(message)s", level=logging.ERROR)
    args.logger = logging.getLogger()
    

    args.source_dir = os.path.dirname( os.path.abspath( os.path.realpath(__file__) ) )
    args.build_dir = os.path.join( args.source_dir, "build")
    if args.prefix:
        args.install_dir = os.path.abspath( os.path.realpath( args.prefix ) )
    else:
        args.install_dir = os.path.join(args.build_dir, "local")
    args.logger.info("Install to the path: [{}]".format(args.install_dir))
    #mkdir_p( args.build_dir )
    mkdir_p( args.install_dir )

    args.install = {}
    install_structure = {"bin": ["bin"],
                         "lib": ["lib"],
                         "libexec": ["libexec"],
                         "libpython": ["lib", "python", "{}_lib".format(config["project"])],
                         "libexec_bin": ["libexec", "bin"],
                         "libexec_mk": ["libexec", "makefiles"]
                        }
    for key, val in install_structure.iteritems():
        args.install[ key ] = os.path.join(args.install_dir, *val)

    args.source = {}
    source_structure = {"python_root": ["python"],
                        "python": ["python", "src"],
                        "pylib":  ["python", "lib"],
                        "cpp": ["cpp"],
                        "makefile": ["makefiles"]}
    for key, val in source_structure.iteritems():
        args.source[ key ] = os.path.join(args.source_dir, "src", *val)

    mkdir_p( args.install["bin"] )
    os.chdir( args.source_dir )
    install_version(args)
    install_python(args)
    install_cpp(args)
    install_makefile(args)


if __name__ == "__main__":
    main()
