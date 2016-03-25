dnl  
dnl A test for determining whether we are on an HP machine, 
dnl using the imake facilities which should define HPArchitecture.
dnl (This is taken from the xgobi Imakefile file and seems reasonable.)
dnl 
dnl Use the Imake facilities to test if HParchitecture
dnl is defined. Return value of a simple rule is different  
dnl depending on whether this is set or not.

dnl If xgobis was licensed under the GPL, then 
dnl one could use the config.guess from Per Bothner and friends
dnl See the R distribution.


AC_DEFUN(AC_XGOBI_HP_TEST,
  [ AC_MSG_CHECKING([whether an HP system])
   cp Imakefile Imakefile.bak 
   cat > Imakefile <<EOF
ok:
#ifdef HPArchitecture
	exit 0
#else
	exit 1
#endif
EOF
    xmkmf >/dev/null 2>&1 
    if make ok >/dev/null 2>&1 ; then 
       forHP=true;
    fi
    mv Imakefile.bak Imakefile
    rm Makefile
    if test -n "${forHP}" ; then
      echo "is HP"
    else
      echo "not HP"
    fi
  ]
)


dnl Test whether the C compiler can generate the dependencies from the source files.
dnl Taken from R with an extension to test for Solaris compile flag -xM.
dnl
AC_DEFUN(XGOBI_C_DEPENDS,
  [ depend_rules_frag=Makefrag.dep
    AC_CACHE_CHECK(
      [how to compiler generates dependency information],
      xgobi_depend_flag,
      [ echo "#include <math.h>" > conftest.${ac_ext}
	if test -n "`${CC} -M conftest.${ac_ext} 2>/dev/null \
		    | grep conftest`"; then
	  xgobi_depend_flag="-M"
	else 
          if test "`${CC} -xM conftest.${ac_ext} 2>/dev/null \
		    | grep conftest`"; then
    	      xgobi_depend_flag="-xM"	
          else
	      xgobi_depend_flag=""	
          fi
	fi
      ])
    if test -n "${xgobi_depend_flag}" ; then
      cat << \EOF > ${depend_rules_frag}
Makefile.dep: $(XGVISSRC) $(XGOBISRC)
	@echo "making $[@] from $^"
	$(CC) ${xgobi_depend_flag} $(ALL_CPPFLAGS) $(XGVISSRC) $(XGOBISRC) > $[@]
EOF
    else
      cat << \EOF > ${depend_rules_frag}
Makefile.dep:
	touch $[@]
EOF
    fi
    AC_SUBST_FILE(depend_rules_frag)
    AC_SUBST(xgobi_depend_flag)
  ])