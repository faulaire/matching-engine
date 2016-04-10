#! /usr/bin/env python
# encoding: utf-8

import os

top = '.'
out = 'build'
APPNAME = 'matching-engine'
VERSION = '0.1'

from waflib.Build import BuildContext

class RunTestCtx(BuildContext):
        cmd = 'run_tests'
        fun = 'run_tests'

def IsClangCompiler(cfg):
   for compiler in cfg.env["CXX"]:
      if compiler.find("clang") != -1:
         return True
   return False

def CheckCompilerVersion(cfg):
    (major, minor, patch) = cfg.env['CC_VERSION']
    version_number = int(major)*100+int(minor)*10+int(patch)
    error_string = "Sorry but to build this project you need to use clang >= 3.6 or g++ >= 4.9"
    if IsClangCompiler(cfg):
        if version_number < 360:
            cfg.fatal(error_string)
    else:
        if version_number < 490:
            cfg.fatal(error_string)


def run_tests(ctx):
    ctx.recurse('common matching-engine')

def options(opt):
    opt.load('compiler_cxx')
    opt.add_option('--release', action='store_true', default=False, help='Compile in release mode')
    opt.add_option('--coverage', action='store_true', default=False, help='Activate coverage')
    opt.add_option('--with_unittest', action='store_true', default=False, help='Activate unittest building')
    opt.add_option('--with_sanitizer', action='store_true', default=False, help='Activate address sanitizer')

def configure(cfg):
    cfg.check_waf_version(mini='1.7.5')

    cfg.load('compiler_cxx')

    CheckCompilerVersion(cfg)

    cfg.check(features='cxx cxxprogram', lib=['pthread'], uselib_store='PTHREAD')
    cfg.check(features='cxx cxxprogram', lib=['m'], uselib_store='M')
    
    cfg.check(features='cxx cxxprogram', lib=['leveldb'], uselib_store='LEVELDB')

    cfg.check(features='cxx cxxprogram', lib=['boost_system'], uselib_store='BOOST_SYSTEM')
    cfg.check(features='cxx cxxprogram', lib=['boost_filesystem'], uselib_store='BOOST_FILESYSTEM')
    cfg.check(features='cxx cxxprogram', lib=['boost_date_time'], uselib_store='BOOST_DATE_TIME')
    cfg.check(features='cxx cxxprogram', lib=['boost_serialization'], uselib_store='BOOST_SERIALIZATION')
    
    cfg.check(header_name='leveldb/db.h', features='cxx cxxprogram')

    cfg.env.with_unittest = cfg.options.with_unittest

    cfg.env.append_value('CXXFLAGS', ['-std=c++14','-W','-Wall','-Wno-unused-local-typedefs', '-D_GLIBCXX_USE_CXX11_ABI=0'])
    
    if IsClangCompiler(cfg):
        # We need to link again libstdc++ and libm with clang
        cfg.env.append_value('LINKFLAGS', ['-lstdc++','-lm'])

    if cfg.options.with_unittest:
        cfg.check(header_name='gtest/gtest.h', features='cxx cxxprogram')
     
    if cfg.options.with_sanitizer:
        cfg.env.append_value('CXXFLAGS', ['-fsanitize=address'])
        cfg.env.append_value('LINKFLAGS', ['-fsanitize=address'])

    if cfg.options.release:
        cfg.env.append_value('CXXFLAGS', ['-O3','-march=native', '-mtune=native'])
    else:
        cfg.env.append_value('CXXFLAGS', ['-ggdb3','-O0','-fno-inline','-fno-omit-frame-pointer'])

        if cfg.options.coverage:
            cfg.env.append_value('CXXFLAGS', ['--coverage','-fPIC'])
            cfg.env.append_value('LINKFLAGS', ['--coverage'])
    
    cfg.recurse('common matching-engine')

def build(bld):
    bld.recurse('common matching-engine')
