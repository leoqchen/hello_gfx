set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin/gl)

set(targets
  #-----------------------------------------------------------------------------------------
  # target_name \; target_sourcecode \; target_compile_options \; target_link_libraries
  #-----------------------------------------------------------------------------------------

  # 等价性测试
  #----------------------------------------------------
  "hello_triangle_gl   \; hello_triangle.cpp"
  "hello_triangle_gles \; hello_triangle.cpp"

  "triangle_glLegacy \; triangle.cpp"
  "triangle_gl       \; triangle.cpp"
  "triangle_gles     \; triangle.cpp"

  "glClear_gl   \; glClear.cpp"
  "glClear_gles \; glClear.cpp"

  "glVertexPointer_glLegacy  \; glVertexPointer.cpp"
  "glVertexPointer_gl        \; glVertexPointer.cpp"
  "glVertexPointer_gles      \; glVertexPointer.cpp"

  "glTexCoordPointer_glLegacy  \; glTexCoordPointer.cpp"
  "glTexCoordPointer_gl        \; glTexCoordPointer.cpp"
  "glTexCoordPointer_gles      \; glTexCoordPointer.cpp"

  "glOrtho_glLegacy  \; glOrtho.cpp"
  "glOrtho_gl        \; glOrtho.cpp"
  "glOrtho_gles      \; glOrtho.cpp"

  "glGenerateMipmap_byGlu_glLegacy \; glGenerateMipmap.cpp"
  "glGenerateMipmap_byCpu_gl       \; glGenerateMipmap.cpp \; ${IS_Cpu}"
  "glGenerateMipmap_byGl_gl        \; glGenerateMipmap.cpp"
  "glGenerateMipmap_byGl_gles      \; glGenerateMipmap.cpp"

  "fbo_ColorAttachRenderbuffer_gl   \; fbo.cpp \; ${IS_ColorAttachRenderbuffer}"
  "fbo_ColorAttachTexture_gl        \; fbo.cpp \; ${IS_ColorAttachTexture}"
  "fbo_ColorAttachRenderbuffer_gles \; fbo.cpp \; ${IS_ColorAttachRenderbuffer}"
  "fbo_ColorAttachTexture_gles      \; fbo.cpp \; ${IS_ColorAttachTexture}"

  # 功能测试
  #----------------------------------------------------
  "glUniform_gl        \; glUniform.cpp"
  "glUniform_gles      \; glUniform.cpp"

  "multiVBO_gl        \; multiVBO.cpp"
  "multiVBO_gles      \; multiVBO.cpp"

  "textureDisplayByFboBlit_gl        \; textureDisplayByFboBlit.cpp"
  "textureDisplayByFboBlit_gles      \; textureDisplayByFboBlit.cpp"

  "textureDisplayByShader_gl        \; textureDisplayByShader.cpp"
  "textureDisplayByShader_gles      \; textureDisplayByShader.cpp"

  "textureRead_gl        \; textureRead.cpp"
  "textureRead_gles      \; textureRead.cpp"

  "textureCopy_gl        \; textureCopy.cpp"

  # perf test 性能测试
  #----------------------------------------------------
  "perf_copytex_glLegacy  \; perf_copytex.cpp"
  "perf_copytex_gl        \; perf_copytex.cpp"
  "perf_copytex_gles      \; perf_copytex.cpp"

  "perf_drawoverhead_glLegacy  \; perf_drawoverhead.cpp"
  "perf_drawoverhead_gl        \; perf_drawoverhead.cpp"
  "perf_drawoverhead_gles      \; perf_drawoverhead.cpp"

  "perf_fbobind_glLegacy  \; perf_fbobind.cpp"
  "perf_fbobind_gl        \; perf_fbobind.cpp"
  "perf_fbobind_gles      \; perf_fbobind.cpp"

  "perf_fill_glLegacy  \; perf_fill_glLegacy.cpp"
  "perf_fill_gl        \; perf_fill_gl.cpp"
  "perf_fill_gles      \; perf_fill_gl.cpp"

  "perf_genmipmap_glLegacy  \; perf_genmipmap.cpp"
  "perf_genmipmap_gl        \; perf_genmipmap.cpp"
  "perf_genmipmap_gles      \; perf_genmipmap.cpp"

  "perf_glslstatechange_glLegacy  \; perf_glslstatechange.cpp"
  "perf_glslstatechange_gl        \; perf_glslstatechange.cpp"
  "perf_glslstatechange_gles      \; perf_glslstatechange.cpp"

  "perf_readpixels_glLegacy  \; perf_readpixels.cpp"
  "perf_readpixels_gl        \; perf_readpixels.cpp"
  "perf_readpixels_gles      \; perf_readpixels.cpp"

  "perf_swapbuffers_glLegacy  \; perf_swapbuffers.cpp"
  "perf_swapbuffers_gl        \; perf_swapbuffers.cpp"
  "perf_swapbuffers_gles      \; perf_swapbuffers.cpp"

  "perf_teximage_glLegacy  \; perf_teximage.cpp"
  "perf_teximage_gl        \; perf_teximage.cpp"
  "perf_teximage_gles      \; perf_teximage.cpp"

  "perf_vbo_glLegacy  \; perf_vbo.cpp"
  "perf_vbo_gl        \; perf_vbo.cpp"
  "perf_vbo_gles      \; perf_vbo.cpp"

  "perf_vertexrate_glLegacy  \; perf_vertexrate.cpp"
  "perf_vertexrate_gl        \; perf_vertexrate.cpp"
  "perf_vertexrate_gles      \; perf_vertexrate.cpp"

  # verify test 验证测试
  #----------------------------------------------------
  "verify_copytex_gl        \; verify_copytex.cpp"

  # Bug复现测试
  #----------------------------------------------------
  "bug_gl_PointSize_glLegacy  \; bug_img/bug_gl_PointSize.cpp"
  "bug_gl_PointSize_gl        \; bug_img/bug_gl_PointSize.cpp"
  "bug_gl_PointSize_gles      \; bug_img/bug_gl_PointSize.cpp"

)

foreach(target ${targets})
  list(LENGTH target length)
  #message("length=${length}")

  list(GET target 0 targetName)
  string(STRIP ${targetName} targetName)

  list(GET target 1 targetSource)
  string(STRIP ${targetSource} targetSource)

  set(targetCompileOptions "")
  if(length GREATER_EQUAL 3)
    list(GET target 2 targetCompileOptions)
    string(STRIP ${targetCompileOptions} targetCompileOptions)
    string(REPLACE " " ";" targetCompileOptions "${targetCompileOptions}")
  endif()

  set(targetLinkLibraries "")
  if(length GREATER_EQUAL 4)
    list(GET target 3 targetLinkLibraries)
    string(STRIP ${targetLinkLibraries} targetLinkLibraries)
    string(REPLACE " " ";" targetLinkLibraries "${targetLinkLibraries}")
  endif()

  if(${targetName} MATCHES "_gl$")
    list(APPEND targetCompileOptions ${IS_Gl})
    list(APPEND targetLinkLibraries myUtils x11Utils glad_gl glUtils_gl glfwUtils_gl eglUtils_gl -lglfw -lX11 -lEGL)
  elseif(${targetName} MATCHES "_glLegacy$")
    list(APPEND targetCompileOptions ${IS_GlLegacy})
    list(APPEND targetLinkLibraries myUtils x11Utils glad_gl glUtils_gl glfwUtils_gl eglUtils_gl -lglfw -lX11 -lEGL -lGLU)
  elseif(${targetName} MATCHES "_gles$")
    list(APPEND targetCompileOptions ${IS_GlEs})
    list(APPEND targetLinkLibraries myUtils x11Utils glad_gles2 glUtils_gles2 glfwUtils_gles2 eglUtils_gles2 -lglfw -lX11 -lEGL)
  endif()

  #message("target=${target}")
  message("targetName=${targetName}, targetSource=${targetSource}, targetCompileOptions=${targetCompileOptions}, targetLinkLibraries=${targetLinkLibraries}")

  add_executable(${targetName} ${targetSource})
  target_compile_options(${targetName} PUBLIC ${targetCompileOptions})
  target_link_libraries(${targetName} -Wl,--start-group ${targetLinkLibraries} -Wl,--end-group)
endforeach(target)
