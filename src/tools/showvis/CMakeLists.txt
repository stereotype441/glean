file (GLOB sources "*.cpp")

add_executable (showvis ${sources})

target_link_libraries (showvis
	dsurf
	lex
	image
	stats
	timer
	${TIFF_LIBRARY}
	${OPENGL_glu_LIBRARY}
	${OPENGL_gl_LIBRARY}
)

if (WIN32)
	target_link_libraries (showvis
		gdi32
		user32
		kernel32
	)
elseif (APPLE)
	target_link_libraries (showvis
		${CARBON_LIBRARY}
	)
else ()
	target_link_libraries (showvis
		${X11_X11_LIB}
	)
endif ()
