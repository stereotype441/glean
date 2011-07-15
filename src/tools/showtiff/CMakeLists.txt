file (GLOB sources "*.cpp")

add_executable (showtiff ${sources})

target_link_libraries (showtiff
	dsurf
	lex
	image
	stats
	timer
	${TIFF_LIBRARY}
	${OPENGL_gl_LIBRARY}
	${OPENGL_glu_LIBRARY}
	${GLUT_glut_LIBRARY}
)

if (WIN32)
	target_link_libraries (showtiff
		gdi32
		user32
		kernel32
	)
elseif (APPLE)
	target_link_libraries (showtiff
		${CARBON_LIBRARY}
	)
else ()
	target_link_libraries (showtiff
		${X11_X11_LIB}
	)
endif ()
