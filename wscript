#! /usr/bin/env python
# encoding: utf-8

import os

top = '.'
out = 'build'
APPNAME = 'matching-engine'
VERSION = '0.1'

from waflib.Build import BuildContext
class RunTestCtx(BuildContext):
        cmd = 'run_test'
        fun = 'run_test'

def IsClangCompiler(cfg):
   for compiler in cfg.env["CXX"]:
      if compiler.find("clang") != -1:
         return True
   return False
   
def run_test(ctx):
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
    cfg.check(features='cxx cxxprogram', lib=['pthread'], uselib_store='PTHREAD')
    cfg.check(features='cxx cxxprogram', lib=['z'], uselib_store='Z')
    cfg.check(features='cxx cxxprogram', lib=['m'], uselib_store='M')
    cfg.check(features='cxx cxxprogram', lib=['dl'], uselib_store='DL')
    
    cfg.check(features='cxx cxxprogram', lib=['leveldb'], uselib_store='LEVELDB')
    cfg.check(header_name='leveldb/db.h', features='cxx cxxprogram')
    
    #TODO : Add a check for boost / ssl 

    cfg.env.with_unittest = cfg.options.with_unittest

    cfg.env.append_value('CXXFLAGS', ['-std=c++1y','-W','-Wall','-Wno-unused-local-typedefs'])
    
    if IsClangCompiler(cfg):
        # We need to link again libstdc++ and libm with clang
        cfg.env.append_value('LINKFLAGS', ['-lstdc++','-lm'])

    if cfg.options.with_unittest:
        cfg.check(header_name='gtest/gtest.h', features='cxx cxxprogram')
        
    if cfg.options.release:
        cfg.env.append_value('CXXFLAGS', ['-O3','-march=amdfam10'])
    else:
        cfg.env.append_value('CXXFLAGS', ['-ggdb3','-O0','-fno-inline','-fno-omit-frame-pointer'])
        
        if cfg.options.with_sanitizer:
            cfg.env.append_value('CXXFLAGS', ['-fsanitize=address'])
            cfg.env.append_value('LINKFLAGS', ['-fsanitize=address'])
        
        if cfg.options.coverage:
            cfg.env.append_value('CXXFLAGS', ['--coverage','-fPIC'])
            cfg.env.append_value('LINKFLAGS', ['--coverage'])
    
    cfg.recurse('common matching-engine')

def build(bld):
    bld.recurse('common matching-engine')
