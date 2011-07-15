file (GLOB sources "*.cpp")

add_executable (glean ${sources})

target_link_libraries (glean
	dsurf
	lex
	image
	stats
	timer
	${TIFF_LIBRARY}
	${OPENGL_gl_LIBRARY}
	${OPENGL_glu_LIBRARY}
)

if (WIN32)
	target_link_libraries (glean
		gdi32
		user32
		kernel32
	)
elseif (APPLE)
	target_link_libraries (glean
		${CARBON_LIBRARY}
	)
else ()
	target_link_libraries (glean
		${X11_X11_LIB}
	)
endif ()
