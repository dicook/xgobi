cmake_minimum_required(VERSION 3.0)

project(xgobi)

if(NOT UNIX)
  message(FATAL_ERROR "xgobi cannot be built on this platform.")
endif()

option(BUILD_SHARED_LIBS OFF "build shared")

find_package(X11 REQUIRED)

find_library(X11_Xaw_LIB
  NAMES Xaw Xaw
  ${X11_LIB_SEARCH_PATH})

if(NOT X11_Xt_FOUND)
  message(FATAL_ERROR "X11_Xt_FOUND not found")
endif()

if(NOT X11_Xaw_LIB)
  message(FATAL_ERROR "X11_Xaw_LIB not found")
endif()
set(XGOBI_X11_LIBRARIES "${X11_LIBRARIES};${X11_Xaw_LIB};${X11_Xt_LIB};${X11_Xmu_LIB}")

include_directories(${X11_INCLUDE_DIR})

message("X11_INCLUDE_DIR=${X11_INCLUDE_DIR}")

message("XGOBI_X11_LIBRARIES=${XGOBI_X11_LIBRARIES}")

set(XGOBI_DEFAULTDIR ${CMAKE_BINARY_DIR})
configure_file(${PROJECT_SOURCE_DIR}/cmake/xgobi_config.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/xgobi_config.h @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(xgobi_common_SRCS
  src/xgobi_init.c
  src/pipeline.c
  src/read_array.c
  src/initialize.c src/widgets.c src/callbacks.c src/getfname.c
  src/xyplot.c
  src/make_axes.c
  src/line_editor.c
  src/missing.c 
  src/spin_cbacks.c
  src/spin.c
  src/scaling.c
  src/scale_cbacks.c
  src/identify.c
  src/var_panel.c
  src/transform.c
  src/sphere.c
  src/stdize.c
  src/inference.c
  src/brush_init.c
  src/brush_cbacks.c
  src/brush.c
  src/paint.c
  src/brush_send.c
  src/help.c
  src/prt_plotwin.c
  src/subset.c
  src/vc_lists.c
  src/exclusion.c
  src/plot_once.c
  src/plot1d.c
  src/texture.c
  src/ash1d.c
  src/move_points.c
  src/show_message.c
  src/smooth.c
  src/smooth_fns.c
  src/smoothsk.c
  src/pspline.c
  src/save_data.c
  src/new_data.c
  src/read_data.c
  src/jitter.c
  src/parcoords.c
  src/utils.c
  src/tour_init.c
  src/tour_util.c
  src/tour.c
  src/tour_cbacks.c
  src/tour_send.c
  src/gt_ctls.c
  src/tour_pp.c
  src/svd.c
  src/kernel.c
  src/legendre.c
  src/hermite.c
  src/natrl_hermite.c
  src/de_indices.c
  src/central_mass.c
  src/holes.c
  src/skewness.c
  src/tour_section.c
  src/corr.c
  src/corr_index.c
  src/corr_pursuit.c
  src/corr_util.c
  src/DrawingA.c
  src/mt19937-1.c
  )

set(xgobi_SRCS
  src/xgobi.c
  )

set(xgvis_SRCS
  src/xgvis.c
   src/xgv_histogram.c
   src/xgv_qsort.c
   src/xgv_mds.c
   src/xgv_read_data.c
   src/xgv_help.c
   src/xgv_stressplot.c
   src/xgv_cbacks.c
   src/xgv_widgets.c
   src/xgv_anchor.c
   )
 
add_definitions (-DUSE_RANDOM)
add_library(xgobi_common OBJECT ${xgobi_common_SRCS} )

add_executable(xgobi ${xgobi_SRCS} $<TARGET_OBJECTS:xgobi_common>)
target_link_libraries(xgobi ${XGOBI_X11_LIBRARIES} m) 

add_executable(xgvis ${xgvis_SRCS} $<TARGET_OBJECTS:xgobi_common>)
target_link_libraries(xgvis ${XGOBI_X11_LIBRARIES} m)
 
install(TARGETS xgobi xgvis DESTINATION bin )

# lint:
# 	lint -DDEFAULTDIR=\"$(XGOBIDIR)\" -I$(IDIR) \
# 		-lm \
# 		/usr/local/pkg/X11/lib/lint/llib-lX11.ln \
# 		/usr/local/pkg/X11/lib/lint/llib-lXaw.ln \
# 		/usr/local/pkg/X11/lib/lint/llib-lXmu.ln \
# 		/usr/local/pkg/X11/lib/lint/llib-lXt.ln \
# 		*.c  >lint.out


