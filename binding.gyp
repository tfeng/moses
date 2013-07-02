{
  "targets": [
    {
      "target_name": "Moses",
      "sources": ["src/Moses.cc",
                  "src/Options.cc"],
      "conditions": [
        ['OS=="linux"', {
          'cflags_cc!': ['-fexceptions']
        }],
        ['OS=="mac"', {
          'xcode_settings': {
            'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
            'OTHER_CFLAGS': ['-Wno-unused-private-field']
          },
          'include_dirs+': ['/opt/translation/moses/include', '/opt/translation/boost/include'],
          'link_settings': {
            'libraries': ['-L/opt/translation/boost/lib', '-lboost_system',
                          '-L/opt/translation/moses/lib', '-lmert_lib', '-lmira_lib', '-lpcfg_common', '-lmoses']
          },
          'defines': ['MAX_NUM_FACTORS=4', 'null=NULL']
        }],
        ['OS=="win"', {
          'msvs_settings': {
            'VCCLCompilerTool': {
              'AdditionalOptions': [ '/EHsc' ]
            }
          }
        }]
      ]
    }
  ]
}
