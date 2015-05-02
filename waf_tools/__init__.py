import os
import fnmatch
import glob

"""
"""

def get_top_level(bld):
    '''
    Get the relative path from the caller wscript to main wscript.
    '''
    import traceback, os
    stack = traceback.extract_stack(limit=2)
    caller = os.path.dirname(stack[0][0])
    root = bld.srcnode.abspath()
    root_to_caller = caller[len(root):].strip(os.path.sep)
    caller_to_root = ''
    for entry in root_to_caller.split(os.path.sep):
            caller_to_root += '..' + os.path.sep
    
    caller_to_root = caller_to_root.rstrip(os.path.sep)
    
    return caller_to_root

def get_module_include_dirs(Context,ModuleName):

    search_path = "%s%sinclude"%(ModuleName,os.sep)
    top_level   = get_top_level(Context)

    list = [ os.path.join(search_path,f) for f in os.listdir(search_path) if os.path.isdir(os.path.join(search_path,f))]
    list.append(search_path)

    list = [ os.path.join(top_level,f) for f in list]

    return list

"""
    Unittest related stuff
    Contain functions to build and run unit tests
"""

def build_tests(Context, deps, include_paths):

    saved_dir = os.getcwd()
    os.chdir( Context.path.relpath() )
    
    source_files = glob.glob("tests/src/*.cpp")
    
    for src in source_files:
        target = "bin"+os.sep+ os.path.basename( os.path.splitext(src)[0] )
        res = Context.program ( source=src, target= target, use=deps, includes=include_paths )
    
    for File in glob.glob("tests/config/*.ini"):
        Context(rule='cp ${SRC} ${TGT}', source=File, target='bin/%s'%os.path.basename(File) )
    
    Context(rule='cp ${SRC} ${TGT}', source="tests/config/pom.xml", target='pom.xml' )
    
    os.chdir( saved_dir )

def run_tests(Context):
    
    bin_dir = Context.out_dir + os.sep + Context.path.relpath() + os.sep + 'bin'
    
    if ( not os.path.isdir( bin_dir ) ):
        # Directory doesn't exist.
        return

    saved_dir = os.getcwd()
    os.chdir( bin_dir )
        
    unit_tests = glob.glob("test_*")
    
    for test in unit_tests:

        JUNIT_RESULT    = "xunit-%s-report.xml"% (test)
        VALGRIND_RESULT = "valgrind-%s-report.xml"% (test)

        Cmd = "valgrind --xml=yes --xml-file=%s ./%s --gtest_output=xml:%s " %( VALGRIND_RESULT, test, JUNIT_RESULT )

        Context.exec_command( Cmd )
    
    os.chdir( Context.top_dir )
    
    Context.exec_command( "gcovr -x -r . > %s/gcovr-report.xml" % (bin_dir) )
    Context.exec_command( "gcovr -r . --html --html-details -o %s/coverage-report.html" % (bin_dir) )

    # Run cppcheck, a general purpose static code checker
    sources_path = "-I%s/include %s/src"%(Context.path.relpath(), Context.path.relpath())
    rats_cmd = "cppcheck --enable=all --inconclusive --xml-version=2 --suppress=missingIncludeSystem %s  2> %s/cppcheck-report.xml"%(sources_path, bin_dir)
    Context.exec_command(rats_cmd)
    
    # Run rats: static code checker focusing on (potential) security problems
    Context.exec_command("rats -w 3 --xml %s > %s/rats-report.xml"%(Context.path.relpath(), bin_dir))
    
    # Run vera++: static code checker focusing on code style issues
    
    # All files but remove unit tests
    files_to_analyze = " find %s -regex \".*\.cpp\|.*\.h\"|grep -v 'test_' "%(Context.path.relpath())
    vera_command     = " vera++ - -showrules -nodup"
    vera_parser_cmd  = " vera++Report2checkstyleReport.perl > %s/vera++-report.xml"%(bin_dir)

    Context.exec_command("bash -c '%s|%s|&%s'"%(files_to_analyze,vera_command,vera_parser_cmd))

    os.chdir( saved_dir )
