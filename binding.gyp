{
  "variables": {
    "BOOST": "$(BOOST)",
    "MOSES": "$(MOSES)",
    "SRILM": "$(SRILM)",
    "IRSTLM": "$(IRSTLM)",
    "MAX_NUM_FACTORS": "4",
    "BOOST_INCLUDE": "<(BOOST)/include",
    "BOOST_LIB": "<(BOOST)/lib",
    "MOSES_INCLUDE": "<(MOSES)/include",
    "MOSES_LIB": "<(MOSES)/lib",
    "SRILM_LIB": "<(SRILM)/lib/i686-m64",
    "IRSTLM_LIB": "<(IRSTLM)/lib"
  },
  "targets": [
    {
      "target_name": "Moses",
      "sources": ["src/Moses.cc",
                  "src/Options.cc"],
      "defines": ["MAX_NUM_FACTORS=<(MAX_NUM_FACTORS)", "null=NULL"],
      "conditions": [
        ["OS=='linux'", {
          "cflags_cc": ["-fexceptions"],
          "include_dirs+": ["<(BOOST_INCLUDE)", "<(MOSES_INCLUDE)"],
          "link_settings": {
            "libraries": ["-L<(BOOST_LIB)", "-lboost_system", "-lboost_thread",
                          "-L<(MOSES_LIB)", "-lmoses", "-lmert_lib", "-lmira_lib", "-lpcfg_common",
                          "-L<(SRILM_LIB)", "-ldstruct", "-lflm", "-llattice", "-loolm", "-lmisc",
                          "-L<(IRSTLM_LIB)", "-lirstlm"]
          }
        }],
        ["OS=='mac'", {
          "variables": {
            "SRILM_LIB": "<(SRILM)/lib/macosx"
          },
          "xcode_settings": {
            "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
            "OTHER_CFLAGS": ["-Wno-unused-private-field"]
          },
          "include_dirs+": ["<(BOOST_INCLUDE)", "<(MOSES_INCLUDE)"],
          "link_settings": {
            "libraries": ["-L<(BOOST_LIB)", "-lboost_system", "-lboost_thread",
                          "-L<(MOSES_LIB)", "-lmoses", "-lmert_lib", "-lmira_lib", "-lpcfg_common",
                          "-L<(SRILM_LIB)", "-ldstruct", "-lflm", "-llattice", "-loolm", "-lmisc",
                          "-L<(IRSTLM_LIB)", "-lirstlm"]
          }
        }],
        ["OS=='win'", {
          "msvs_settings": {
            "VCCLCompilerTool": {
              "AdditionalOptions": [ "/EHsc" ]
            }
          }
        }]
      ]
    }
  ]
}
