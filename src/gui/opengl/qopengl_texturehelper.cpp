/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
*
* Copyright (c) 2013 Klarälvdalens Datakonsult AB, a KDAB Group company
* Copyright (c) 2015 The Qt Company Ltd.
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
*
* This file is part of CopperSpice.
*
* CopperSpice is free software. You can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* https://www.gnu.org/licenses/
*
***********************************************************************/

#include <qopengl_texturehelper_p.h>

#include <qopenglcontext.h>

#include <qopengl_extensions_p.h>

template <typename T, typename U>
std::enable_if_t<sizeof(T) == sizeof(U) &&
   std::is_trivially_copyable_v<T> && std::is_trivially_copyable_v<U>, T>
cs_bitCast(const U &input) noexcept
{
   static_assert(std::is_trivially_constructible_v<T>);

   T retval;
   std::memcpy(&retval, &input, sizeof(U));

   return retval;
}

QOpenGLTextureHelper::QOpenGLTextureHelper(QOpenGLContext *context)
{
    // Resolve EXT_direct_state_access entry points if present.

    // However, disable it on some systems where DSA is known to be unreliable.
    bool allowDSA = true;
    const char *renderer = reinterpret_cast<const char *>(context->functions()->glGetString(GL_RENDERER));

    // QTBUG-40653, QTBUG-44988
    if (renderer && strstr(renderer, "AMD Radeon HD"))
        allowDSA = false;

    if (allowDSA && ! context->isOpenGLES() && context->hasExtension("GL_EXT_direct_state_access")) {
        TextureParameteriEXT = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint, GLenum, GLenum,
              GLint )>(context->getProcAddress("glTextureParameteriEXT"));

        TextureParameterivEXT = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint, GLenum, GLenum,
              const GLint *)>(context->getProcAddress("glTextureParameterivEXT"));

        TextureParameterfEXT = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint, GLenum, GLenum,
              GLfloat )>(context->getProcAddress("glTextureParameterfEXT"));

        TextureParameterfvEXT = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint, GLenum, GLenum,
              const GLfloat *)>(context->getProcAddress("glTextureParameterfvEXT"));

        GenerateTextureMipmapEXT = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint,
              GLenum )>(context->getProcAddress("glGenerateTextureMipmapEXT"));

        TextureStorage3DEXT = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint, GLenum, GLsizei,
              GLenum, GLsizei, GLsizei, GLsizei )>(context->getProcAddress("glTextureStorage3DEXT"));

        TextureStorage2DEXT = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint, GLenum, GLsizei,
              GLenum, GLsizei, GLsizei )>(context->getProcAddress("glTextureStorage2DEXT"));

        TextureStorage1DEXT = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint, GLenum, GLsizei,
              GLenum, GLsizei )>(context->getProcAddress("glTextureStorage1DEXT"));

        TextureStorage3DMultisampleEXT = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint, GLenum, GLsizei, GLenum,
              GLsizei, GLsizei, GLsizei, GLboolean )>(context->getProcAddress("glTextureStorage3DMultisampleEXT"));

        TextureStorage2DMultisampleEXT = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint, GLenum, GLsizei,
              GLenum, GLsizei, GLsizei, GLboolean )>(context->getProcAddress("glTextureStorage2DMultisampleEXT"));

        TextureImage3DEXT = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint, GLenum, GLint, GLenum, GLsizei,
               GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid *)>(context->getProcAddress("glTextureImage3DEXT"));

        TextureImage2DEXT = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint, GLenum, GLint, GLenum, GLsizei,
               GLsizei, GLint, GLenum, GLenum, const GLvoid *)>(context->getProcAddress("glTextureImage2DEXT"));

        TextureImage1DEXT = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint, GLenum, GLint, GLenum, GLsizei,
               GLint, GLenum, GLenum, const GLvoid *)>(context->getProcAddress("glTextureImage1DEXT"));

        TextureSubImage3DEXT = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint, GLenum, GLint, GLint, GLint, GLint,
               GLsizei, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *)>(context->getProcAddress("glTextureSubImage3DEXT"));

        TextureSubImage2DEXT = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint, GLenum, GLint, GLint, GLint,
               GLsizei, GLsizei, GLenum, GLenum, const GLvoid *)>(context->getProcAddress("glTextureSubImage2DEXT"));

        TextureSubImage1DEXT = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint, GLenum, GLint, GLint, GLsizei,
               GLenum, GLenum, const GLvoid *)>(context->getProcAddress("glTextureSubImage1DEXT"));

        CompressedTextureSubImage1DEXT = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint, GLenum, GLint, GLint,
               GLsizei, GLenum, GLsizei, const GLvoid *)>(context->getProcAddress("glCompressedTextureSubImage1DEXT"));

        CompressedTextureSubImage2DEXT = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint, GLenum, GLint, GLint,
               GLint, GLsizei, GLsizei, GLenum, GLsizei, const GLvoid *)>(context->getProcAddress("glCompressedTextureSubImage2DEXT"));

        CompressedTextureSubImage3DEXT = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint, GLenum, GLint, GLint,
               GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLsizei,
               const GLvoid *)>(context->getProcAddress("glCompressedTextureSubImage3DEXT"));

        CompressedTextureImage1DEXT = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint, GLenum, GLint, GLenum,
               GLsizei, GLint, GLsizei, const GLvoid *)>(context->getProcAddress("glCompressedTextureImage1DEXT"));

        CompressedTextureImage2DEXT = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , GLint , GLenum,
               GLsizei, GLsizei, GLint, GLsizei, const GLvoid *)>(context->getProcAddress("glCompressedTextureImage2DEXT"));

        CompressedTextureImage3DEXT = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint, GLenum, GLint, GLenum,
               GLsizei, GLsizei, GLsizei, GLint, GLsizei, const GLvoid *)>(context->getProcAddress("glCompressedTextureImage3DEXT"));

        // Use the real DSA functions
        TextureParameteri = &QOpenGLTextureHelper::dsa_TextureParameteri;
        TextureParameteriv = &QOpenGLTextureHelper::dsa_TextureParameteriv;
        TextureParameterf = &QOpenGLTextureHelper::dsa_TextureParameterf;
        TextureParameterfv = &QOpenGLTextureHelper::dsa_TextureParameterfv;
        GenerateTextureMipmap = &QOpenGLTextureHelper::dsa_GenerateTextureMipmap;
        TextureStorage3D = &QOpenGLTextureHelper::dsa_TextureStorage3D;
        TextureStorage2D = &QOpenGLTextureHelper::dsa_TextureStorage2D;
        TextureStorage1D = &QOpenGLTextureHelper::dsa_TextureStorage1D;
        TextureStorage3DMultisample = &QOpenGLTextureHelper::dsa_TextureStorage3DMultisample;
        TextureStorage2DMultisample = &QOpenGLTextureHelper::dsa_TextureStorage2DMultisample;
        TextureImage3D = &QOpenGLTextureHelper::dsa_TextureImage3D;
        TextureImage2D = &QOpenGLTextureHelper::dsa_TextureImage2D;
        TextureImage1D = &QOpenGLTextureHelper::dsa_TextureImage1D;
        TextureSubImage3D = &QOpenGLTextureHelper::dsa_TextureSubImage3D;
        TextureSubImage2D = &QOpenGLTextureHelper::dsa_TextureSubImage2D;
        TextureSubImage1D = &QOpenGLTextureHelper::dsa_TextureSubImage1D;
        CompressedTextureSubImage1D = &QOpenGLTextureHelper::dsa_CompressedTextureSubImage1D;
        CompressedTextureSubImage2D = &QOpenGLTextureHelper::dsa_CompressedTextureSubImage2D;
        CompressedTextureSubImage3D = &QOpenGLTextureHelper::dsa_CompressedTextureSubImage3D;
        CompressedTextureImage1D = &QOpenGLTextureHelper::dsa_CompressedTextureImage1D;
        CompressedTextureImage2D = &QOpenGLTextureHelper::dsa_CompressedTextureImage2D;
        CompressedTextureImage3D = &QOpenGLTextureHelper::dsa_CompressedTextureImage3D;
    } else {
        // Use our own DSA emulation
        TextureParameteri = &QOpenGLTextureHelper::qt_TextureParameteri;
        TextureParameteriv = &QOpenGLTextureHelper::qt_TextureParameteriv;
        TextureParameterf = &QOpenGLTextureHelper::qt_TextureParameterf;
        TextureParameterfv = &QOpenGLTextureHelper::qt_TextureParameterfv;
        GenerateTextureMipmap = &QOpenGLTextureHelper::qt_GenerateTextureMipmap;
        TextureStorage3D = &QOpenGLTextureHelper::qt_TextureStorage3D;
        TextureStorage2D = &QOpenGLTextureHelper::qt_TextureStorage2D;
        TextureStorage1D = &QOpenGLTextureHelper::qt_TextureStorage1D;
        TextureStorage3DMultisample = &QOpenGLTextureHelper::qt_TextureStorage3DMultisample;
        TextureStorage2DMultisample = &QOpenGLTextureHelper::qt_TextureStorage2DMultisample;
        TextureImage3D = &QOpenGLTextureHelper::qt_TextureImage3D;
        TextureImage2D = &QOpenGLTextureHelper::qt_TextureImage2D;
        TextureImage1D = &QOpenGLTextureHelper::qt_TextureImage1D;
        TextureSubImage3D = &QOpenGLTextureHelper::qt_TextureSubImage3D;
        TextureSubImage2D = &QOpenGLTextureHelper::qt_TextureSubImage2D;
        TextureSubImage1D = &QOpenGLTextureHelper::qt_TextureSubImage1D;
        CompressedTextureSubImage1D = &QOpenGLTextureHelper::qt_CompressedTextureSubImage1D;
        CompressedTextureSubImage2D = &QOpenGLTextureHelper::qt_CompressedTextureSubImage2D;
        CompressedTextureSubImage3D = &QOpenGLTextureHelper::qt_CompressedTextureSubImage3D;
        CompressedTextureImage1D = &QOpenGLTextureHelper::qt_CompressedTextureImage1D;
        CompressedTextureImage2D = &QOpenGLTextureHelper::qt_CompressedTextureImage2D;
        CompressedTextureImage3D = &QOpenGLTextureHelper::qt_CompressedTextureImage3D;
    }

    // Some DSA functions are part of NV_texture_multisample instead
    if (! context->isOpenGLES()
        && context->hasExtension("GL_NV_texture_multisample")) {

        TextureImage3DMultisampleNV = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint, GLenum, GLsizei, GLint,
              GLsizei, GLsizei, GLsizei, GLboolean )>(context->getProcAddress("glTextureImage3DMultisampleNV"));

        TextureImage2DMultisampleNV = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint, GLenum, GLsizei, GLint,
              GLsizei, GLsizei, GLboolean )>(context->getProcAddress("glTextureImage2DMultisampleNV"));

        TextureImage3DMultisample = &QOpenGLTextureHelper::dsa_TextureImage3DMultisample;
        TextureImage2DMultisample = &QOpenGLTextureHelper::dsa_TextureImage2DMultisample;

    } else {
        TextureImage3DMultisample = &QOpenGLTextureHelper::qt_TextureImage3DMultisample;
        TextureImage2DMultisample = &QOpenGLTextureHelper::qt_TextureImage2DMultisample;
    }

    // wglGetProcAddress should not be used to (and indeed will not) load OpenGL <= 1.1 functions.
    // Hence, we resolve them "the hard way"

#if defined(Q_OS_WIN) && ! defined(QT_OPENGL_ES_2)
    HMODULE handle = static_cast<HMODULE>(QOpenGLContext::openGLModuleHandle());

    if (! handle) {
        handle = GetModuleHandleA("opengl32.dll");
    }

    // OpenGL 1.0
    GetIntegerv = cs_bitCast<void (QOPENGLF_APIENTRYP)(GLenum, GLint *)>(GetProcAddress(handle, "glGetIntegerv"));
    GetBooleanv = cs_bitCast<void (QOPENGLF_APIENTRYP)(GLenum, GLboolean *)>(GetProcAddress(handle, "glGetBooleanv"));
    PixelStorei = cs_bitCast<void (QOPENGLF_APIENTRYP)(GLenum, GLint )>(GetProcAddress(handle, "glPixelStorei"));

    GetTexLevelParameteriv = cs_bitCast<void (QOPENGLF_APIENTRYP)(GLenum, GLint, GLenum,
          GLint *)>(GetProcAddress(handle, "glGetTexLevelParameteriv"));

    GetTexLevelParameterfv = cs_bitCast<void (QOPENGLF_APIENTRYP)(GLenum, GLint, GLenum,
          GLfloat *)>(GetProcAddress(handle, "glGetTexLevelParameterfv"));

    GetTexParameteriv = cs_bitCast<void (QOPENGLF_APIENTRYP)(GLenum, GLenum,
           GLint *)>(GetProcAddress(handle, "glGetTexParameteriv"));

    GetTexParameterfv = cs_bitCast<void (QOPENGLF_APIENTRYP)(GLenum, GLenum,
           GLfloat *)>(GetProcAddress(handle, "glGetTexParameterfv"));

    GetTexImage = cs_bitCast<void (QOPENGLF_APIENTRYP)(GLenum, GLint, GLenum, GLenum,
           GLvoid *)>(GetProcAddress(handle, "glGetTexImage"));

    TexImage2D = cs_bitCast<void (QOPENGLF_APIENTRYP)(GLenum, GLint, GLint, GLsizei,
           GLsizei, GLint, GLenum, GLenum, const GLvoid *)>(GetProcAddress(handle, "glTexImage2D"));

    TexImage1D = cs_bitCast<void (QOPENGLF_APIENTRYP)(GLenum, GLint, GLint, GLsizei, GLint, GLenum,
           GLenum, const GLvoid *)>(GetProcAddress(handle, "glTexImage1D"));

    TexParameteriv = cs_bitCast<void (QOPENGLF_APIENTRYP)(GLenum, GLenum, const GLint *)>(GetProcAddress(handle, "glTexParameteriv"));
    TexParameteri  = cs_bitCast<void (QOPENGLF_APIENTRYP)(GLenum, GLenum, GLint )>(GetProcAddress(handle, "glTexParameteri"));
    TexParameterfv = cs_bitCast<void (QOPENGLF_APIENTRYP)(GLenum, GLenum, const GLfloat *)>(GetProcAddress(handle, "glTexParameterfv"));
    TexParameterf  = cs_bitCast<void (QOPENGLF_APIENTRYP)(GLenum, GLenum, GLfloat )>(GetProcAddress(handle, "glTexParameterf"));

    // OpenGL 1.1
    GenTextures    = cs_bitCast<void (QOPENGLF_APIENTRYP)(GLsizei, GLuint *)>(GetProcAddress(handle, "glGenTextures"));
    DeleteTextures = cs_bitCast<void (QOPENGLF_APIENTRYP)(GLsizei, const GLuint *)>(GetProcAddress(handle, "glDeleteTextures"));
    BindTexture    = cs_bitCast<void (QOPENGLF_APIENTRYP)(GLenum, GLuint )>(GetProcAddress(handle, "glBindTexture"));

    TexSubImage2D  = cs_bitCast<void (QOPENGLF_APIENTRYP)(GLenum, GLint, GLint, GLint,
           GLsizei, GLsizei, GLenum, GLenum, const GLvoid *)>(GetProcAddress(handle, "glTexSubImage2D"));

    TexSubImage1D  = cs_bitCast<void (QOPENGLF_APIENTRYP)(GLenum, GLint, GLint, GLsizei,
           GLenum, GLenum, const GLvoid *)>(GetProcAddress(handle, "glTexSubImage1D"));

#elif defined(QT_OPENGL_ES_2)
    // Here we are targeting OpenGL ES 2.0+ only. This is likely using EGL, where,
    // similarly to WGL, non-extension functions (i.e. any function that is part of the
    // GLES spec) *may* not be queried via eglGetProcAddress.

    // OpenGL 1.0
    GetIntegerv = ::glGetIntegerv;
    GetBooleanv = ::glGetBooleanv;
    PixelStorei = ::glPixelStorei;
    GetTexLevelParameteriv = 0;
    GetTexLevelParameterfv = 0;
    GetTexParameteriv = ::glGetTexParameteriv;
    GetTexParameterfv = ::glGetTexParameterfv;
    GetTexImage = 0;

    TexImage2D = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum, GLint, GLint, GLsizei, GLsizei,
           GLint, GLenum, GLenum, const GLvoid *)>(::glTexImage2D);

    TexImage1D = 0;
    TexParameteriv = ::glTexParameteriv;
    TexParameteri = ::glTexParameteri;
    TexParameterfv = ::glTexParameterfv;
    TexParameterf = ::glTexParameterf;

    // OpenGL 1.1
    GenTextures = ::glGenTextures;
    DeleteTextures = ::glDeleteTextures;
    BindTexture = ::glBindTexture;
    TexSubImage2D = ::glTexSubImage2D;
    TexSubImage1D = 0;

    // OpenGL 1.3
    GetCompressedTexImage = 0;
    CompressedTexSubImage1D = 0;
    CompressedTexSubImage2D = ::glCompressedTexSubImage2D;
    CompressedTexImage1D = 0;
    CompressedTexImage2D = ::glCompressedTexImage2D;
    ActiveTexture = ::glActiveTexture;

    // OpenGL 3.0
    GenerateMipmap = ::glGenerateMipmap;

    // OpenGL 3.2
    TexImage3DMultisample = 0;
    TexImage2DMultisample = 0;

    // OpenGL 4.2
    QOpenGLContext *ctx = QOpenGLContext::currentContext();
    if (ctx->format().majorVersion() >= 3) {
        // OpenGL ES 3.0+ has immutable storage for 2D and 3D at least.
        QOpenGLES3Helper *es3 = static_cast<QOpenGLExtensions *>(ctx->functions())->gles3Helper();
        TexStorage3D = es3->TexStorage3D;
        TexStorage2D = es3->TexStorage2D;
    } else {
        TexStorage3D = 0;
        TexStorage2D = 0;
    }
    TexStorage1D = 0;

    // OpenGL 4.3
    TexStorage3DMultisample = 0;
    TexStorage2DMultisample = 0;
    TexBufferRange = 0;
    TextureView = 0;

#else

    // OpenGL 1.0
    GetIntegerv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint *)>(context->getProcAddress("glGetIntegerv"));
    GetBooleanv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLboolean *)>(context->getProcAddress("glGetBooleanv"));

    PixelStorei = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint )>(context->getProcAddress("glPixelStorei"));

    GetTexLevelParameteriv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint , GLenum , GLint *)>(context->getProcAddress("glGetTexLevelParameteriv"));

    GetTexLevelParameterfv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint , GLenum , GLfloat *)>(context->getProcAddress("glGetTexLevelParameterfv"));

    GetTexParameteriv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLint *)>(context->getProcAddress("glGetTexParameteriv"));

    GetTexParameterfv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLfloat *)>(context->getProcAddress("glGetTexParameterfv"));

    GetTexImage = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint , GLenum , GLenum , GLvoid *)>(context->getProcAddress("glGetTexImage"));

    TexImage2D = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint , GLint , GLsizei , GLsizei , GLint , GLenum , GLenum , const GLvoid *)>(context->getProcAddress("glTexImage2D"));

    TexImage1D = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint , GLint , GLsizei , GLint , GLenum , GLenum , const GLvoid *)>(context->getProcAddress("glTexImage1D"));

    TexParameteriv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , const GLint *)>(context->getProcAddress("glTexParameteriv"));

    TexParameteri = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLint )>(context->getProcAddress("glTexParameteri"));

    TexParameterfv = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , const GLfloat *)>(context->getProcAddress("glTexParameterfv"));

    TexParameterf = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLfloat )>(context->getProcAddress("glTexParameterf"));

    // OpenGL 1.1
    GenTextures = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLsizei , GLuint *)>(context->getProcAddress("glGenTextures"));
    DeleteTextures = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLsizei , const GLuint *)>(context->getProcAddress("glDeleteTextures"));

    BindTexture = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLuint )>(context->getProcAddress("glBindTexture"));

    TexSubImage2D = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint , GLint , GLint , GLsizei , GLsizei , GLenum , GLenum , const GLvoid *)>(context->getProcAddress("glTexSubImage2D"));

    TexSubImage1D = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint , GLint , GLsizei , GLenum , GLenum , const GLvoid *)>(context->getProcAddress("glTexSubImage1D"));
#endif

    if (context->isOpenGLES() && context->hasExtension("GL_OES_texture_3D")) {
        TexImage3D = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum, GLint, GLint, GLsizei, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid*)>(context->getProcAddress("glTexImage3DOES"));

        TexSubImage3D = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLenum, const GLvoid*)>(context->getProcAddress("glTexSubImage3DOES"));
        CompressedTexImage3D = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum, GLint, GLenum, GLsizei, GLsizei, GLsizei, GLint, GLsizei, const GLvoid*)>(context->getProcAddress("glCompressedTexImage3DOES"));

        CompressedTexSubImage3D = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLsizei, const GLvoid*)>(context->getProcAddress("glCompressedTexSubImage3DOES"));

    } else {
        QOpenGLContext *ctx = QOpenGLContext::currentContext();
        if (ctx->isOpenGLES() && ctx->format().majorVersion() >= 3) {
            // OpenGL ES 3.0+ has glTexImage3D.
            QOpenGLES3Helper *es3 = static_cast<QOpenGLExtensions *>(ctx->functions())->gles3Helper();
            TexImage3D = es3->TexImage3D;
            TexSubImage3D = es3->TexSubImage3D;
            CompressedTexImage3D = es3->CompressedTexImage3D;
            CompressedTexSubImage3D = es3->CompressedTexSubImage3D;
        } else {
            // OpenGL 1.2
            TexImage3D = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint , GLint , GLsizei , GLsizei , GLsizei , GLint , GLenum , GLenum , const GLvoid *)>(context->getProcAddress("glTexImage3D"));

            TexSubImage3D = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint , GLint , GLint , GLint , GLsizei , GLsizei , GLsizei , GLenum , GLenum , const GLvoid *)>(context->getProcAddress("glTexSubImage3D"));

            // OpenGL 1.3
            CompressedTexImage3D = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint , GLenum , GLsizei , GLsizei , GLsizei , GLint , GLsizei , const GLvoid *)>(context->getProcAddress("glCompressedTexImage3D"));

            CompressedTexSubImage3D = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint , GLint , GLint , GLint , GLsizei , GLsizei , GLsizei , GLenum , GLsizei , const GLvoid *)>(context->getProcAddress("glCompressedTexSubImage3D"));
        }
    }

#ifndef QT_OPENGL_ES_2
    // OpenGL 1.3
    GetCompressedTexImage = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint , GLvoid *)>(context->getProcAddress("glGetCompressedTexImage"));

    CompressedTexSubImage1D = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint , GLint , GLsizei , GLenum , GLsizei , const GLvoid *)>(context->getProcAddress("glCompressedTexSubImage1D"));

    CompressedTexSubImage2D = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint , GLint , GLint , GLsizei , GLsizei , GLenum , GLsizei , const GLvoid *)>(context->getProcAddress("glCompressedTexSubImage2D"));

    CompressedTexImage1D = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint , GLenum , GLsizei , GLint , GLsizei , const GLvoid *)>(context->getProcAddress("glCompressedTexImage1D"));

    CompressedTexImage2D = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLint , GLenum , GLsizei , GLsizei , GLint , GLsizei , const GLvoid *)>(context->getProcAddress("glCompressedTexImage2D"));

    ActiveTexture = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum )>(context->getProcAddress("glActiveTexture"));

    // OpenGL 3.0
    GenerateMipmap = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum )>(context->getProcAddress("glGenerateMipmap"));

    // OpenGL 3.2
    TexImage3DMultisample = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLsizei , GLint , GLsizei , GLsizei , GLsizei , GLboolean )>(context->getProcAddress("glTexImage3DMultisample"));

    TexImage2DMultisample = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLsizei , GLint , GLsizei , GLsizei , GLboolean )>(context->getProcAddress("glTexImage2DMultisample"));

    // OpenGL 4.2
    TexStorage3D = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLsizei , GLenum , GLsizei , GLsizei , GLsizei )>(context->getProcAddress("glTexStorage3D"));

    TexStorage2D = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLsizei , GLenum , GLsizei , GLsizei )>(context->getProcAddress("glTexStorage2D"));

    TexStorage1D = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLsizei , GLenum , GLsizei )>(context->getProcAddress("glTexStorage1D"));

    // OpenGL 4.3
    TexStorage3DMultisample = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLsizei , GLenum , GLsizei , GLsizei , GLsizei , GLboolean )>(context->getProcAddress("glTexStorage3DMultisample"));

    TexStorage2DMultisample = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLsizei , GLenum , GLsizei , GLsizei , GLboolean )>(context->getProcAddress("glTexStorage2DMultisample"));

    TexBufferRange = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLenum , GLenum , GLuint , GLintptr , GLsizeiptr )>(context->getProcAddress("glTexBufferRange"));

    TextureView = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint , GLenum , GLuint , GLenum , GLuint , GLuint , GLuint , GLuint )>(context->getProcAddress("glTextureView"));
#endif
}

void QOpenGLTextureHelper::dsa_TextureParameteri(GLuint texture, GLenum target, GLenum bindingTarget, GLenum pname, GLint param)
{
    (void) bindingTarget;
    TextureParameteriEXT(texture, target, pname, param);
}

void QOpenGLTextureHelper::dsa_TextureParameteriv(GLuint texture, GLenum target, GLenum bindingTarget, GLenum pname, const GLint *params)
{
    (void) bindingTarget;
    TextureParameterivEXT(texture, target, pname, params);
}

void QOpenGLTextureHelper::dsa_TextureParameterf(GLuint texture, GLenum target, GLenum bindingTarget, GLenum pname, GLfloat param)
{
    (void) bindingTarget;
    TextureParameterfEXT(texture, target, pname, param);
}

void QOpenGLTextureHelper::dsa_TextureParameterfv(GLuint texture, GLenum target, GLenum bindingTarget, GLenum pname, const GLfloat *params)
{
    (void) bindingTarget;
    TextureParameterfvEXT(texture, target, pname, params);
}

void QOpenGLTextureHelper::dsa_GenerateTextureMipmap(GLuint texture, GLenum target, GLenum bindingTarget)
{
    (void) bindingTarget;
    GenerateTextureMipmapEXT(texture, target);
}

void QOpenGLTextureHelper::dsa_TextureStorage3D(GLuint texture, GLenum target, GLenum bindingTarget, GLsizei levels, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei depth)
{
    (void) bindingTarget;
    TextureStorage3DEXT(texture, target, levels, internalFormat, width, height, depth);
}

void QOpenGLTextureHelper::dsa_TextureStorage2D(GLuint texture, GLenum target, GLenum bindingTarget, GLsizei levels, GLenum internalFormat, GLsizei width, GLsizei height)
{
    (void) bindingTarget;
    TextureStorage2DEXT(texture, target, levels, internalFormat, width, height);
}

void QOpenGLTextureHelper::dsa_TextureStorage1D(GLuint texture, GLenum target, GLenum bindingTarget, GLsizei levels, GLenum internalFormat, GLsizei width)
{
    (void) bindingTarget;
    TextureStorage1DEXT(texture, target, levels, internalFormat, width);
}

void QOpenGLTextureHelper::dsa_TextureStorage3DMultisample(GLuint texture, GLenum target, GLenum bindingTarget, GLsizei samples, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedSampleLocations)
{
    (void) bindingTarget;
    TextureStorage3DMultisampleEXT(texture, target, samples, internalFormat, width, height, depth, fixedSampleLocations);
}

void QOpenGLTextureHelper::dsa_TextureStorage2DMultisample(GLuint texture, GLenum target, GLenum bindingTarget, GLsizei samples, GLenum internalFormat, GLsizei width, GLsizei height, GLboolean fixedSampleLocations)
{
    (void) bindingTarget;
    TextureStorage2DMultisampleEXT(texture, target, samples, internalFormat, width, height, fixedSampleLocations);
}

void QOpenGLTextureHelper::dsa_TextureImage3D(GLuint texture, GLenum target, GLenum bindingTarget, GLint level, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
    (void) bindingTarget;
    TextureImage3DEXT(texture, target, level, internalFormat, width, height, depth, border, format, type, pixels);
}

void QOpenGLTextureHelper::dsa_TextureImage2D(GLuint texture, GLenum target, GLenum bindingTarget, GLint level, GLenum internalFormat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
    (void) bindingTarget;
    TextureImage2DEXT(texture, target, level, internalFormat, width, height, border, format, type, pixels);
}

void QOpenGLTextureHelper::dsa_TextureImage1D(GLuint texture, GLenum target, GLenum bindingTarget, GLint level, GLenum internalFormat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
    (void) bindingTarget;
    TextureImage1DEXT(texture, target, level, internalFormat, width, border, format, type, pixels);
}

void QOpenGLTextureHelper::dsa_TextureSubImage3D(GLuint texture, GLenum target, GLenum bindingTarget, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels)
{
    (void) bindingTarget;
    TextureSubImage3DEXT(texture, target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
}

void QOpenGLTextureHelper::dsa_TextureSubImage2D(GLuint texture, GLenum target, GLenum bindingTarget, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels)
{
    (void) bindingTarget;
    TextureSubImage2DEXT(texture, target, level, xoffset, yoffset, width, height, format, type, pixels);
}

void QOpenGLTextureHelper::dsa_TextureSubImage1D(GLuint texture, GLenum target, GLenum bindingTarget, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels)
{
    (void) bindingTarget;
    TextureSubImage1DEXT(texture, target, level, xoffset, width, format, type, pixels);
}

void QOpenGLTextureHelper::dsa_TextureImage3DMultisample(GLuint texture, GLenum target, GLenum bindingTarget, GLsizei samples, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedSampleLocations)
{
    (void) bindingTarget;
    TextureImage3DMultisampleNV(texture, target, samples, internalFormat, width, height, depth, fixedSampleLocations);
}

void QOpenGLTextureHelper::dsa_TextureImage2DMultisample(GLuint texture, GLenum target, GLenum bindingTarget, GLsizei samples, GLint internalFormat, GLsizei width, GLsizei height, GLboolean fixedSampleLocations)
{
    (void) bindingTarget;
    TextureImage2DMultisampleNV(texture, target, samples, internalFormat, width, height, fixedSampleLocations);
}

void QOpenGLTextureHelper::dsa_CompressedTextureSubImage1D(GLuint texture, GLenum target, GLenum bindingTarget, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *bits)
{
    (void) bindingTarget;
    CompressedTextureSubImage1DEXT(texture, target, level, xoffset, width, format, imageSize, bits);
}

void QOpenGLTextureHelper::dsa_CompressedTextureSubImage2D(GLuint texture, GLenum target, GLenum bindingTarget, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *bits)
{
    (void) bindingTarget;
    CompressedTextureSubImage2DEXT(texture, target, level, xoffset, yoffset, width, height, format, imageSize, bits);
}

void QOpenGLTextureHelper::dsa_CompressedTextureSubImage3D(GLuint texture, GLenum target, GLenum bindingTarget, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *bits)
{
    (void) bindingTarget;
    CompressedTextureSubImage3DEXT(texture, target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, bits);
}

void QOpenGLTextureHelper::dsa_CompressedTextureImage1D(GLuint texture, GLenum target, GLenum bindingTarget, GLint level, GLenum internalFormat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *bits)
{
    (void) bindingTarget;
    CompressedTextureImage1DEXT(texture, target, level, internalFormat, width, border, imageSize, bits);
}

void QOpenGLTextureHelper::dsa_CompressedTextureImage2D(GLuint texture, GLenum target, GLenum bindingTarget, GLint level, GLenum internalFormat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *bits)
{
    (void) bindingTarget;
    CompressedTextureImage2DEXT(texture, target, level, internalFormat, width, height, border, imageSize, bits);
}

void QOpenGLTextureHelper::dsa_CompressedTextureImage3D(GLuint texture, GLenum target, GLenum bindingTarget, GLint level, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *bits)
{
    (void) bindingTarget;
    CompressedTextureImage3DEXT(texture, target, level, internalFormat, width, height, depth, border, imageSize, bits);
}

namespace {

class TextureBinder
{
public:
    TextureBinder(QOpenGLTextureHelper *textureFunctions, GLuint texture, GLenum target, GLenum bindingTarget)
    : m_textureFunctions(textureFunctions)
    {
        // For cubemaps we can't use the standard DSA emulation as it is illegal to
        // try to bind a texture to one of the cubemap face targets. So we force the
        // target and binding target to the cubemap values in this case.
        switch (target) {
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
        case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
        case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
        case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
            bindingTarget = GL_TEXTURE_BINDING_CUBE_MAP;
            m_target = GL_TEXTURE_CUBE_MAP;
            break;

        default:
            m_target = target;
            break;
        }

        m_textureFunctions->glGetIntegerv(bindingTarget, &m_oldTexture);
        m_textureFunctions->glBindTexture(m_target, texture);
    }

    ~TextureBinder()
    {
        m_textureFunctions->glBindTexture(m_target, m_oldTexture);
    }

private:
    QOpenGLTextureHelper *m_textureFunctions;
    GLenum m_target;
    GLint m_oldTexture;
};

} // namespace

void QOpenGLTextureHelper::qt_TextureParameteri(GLuint texture, GLenum target, GLenum bindingTarget, GLenum pname, GLint param)
{
    TextureBinder binder(this, texture, target, bindingTarget);
    glTexParameteri(target, pname, param);
}

void QOpenGLTextureHelper::qt_TextureParameteriv(GLuint texture, GLenum target, GLenum bindingTarget, GLenum pname, const GLint *params)
{
    TextureBinder binder(this, texture, target, bindingTarget);
    glTexParameteriv(target, pname, params);
}

void QOpenGLTextureHelper::qt_TextureParameterf(GLuint texture, GLenum target, GLenum bindingTarget, GLenum pname, GLfloat param)
{
    TextureBinder binder(this, texture, target, bindingTarget);
    glTexParameterf(target, pname, param);
}

void QOpenGLTextureHelper::qt_TextureParameterfv(GLuint texture, GLenum target, GLenum bindingTarget, GLenum pname, const GLfloat *params)
{
    TextureBinder binder(this, texture, target, bindingTarget);
    glTexParameterfv(target, pname, params);
}

void QOpenGLTextureHelper::qt_GenerateTextureMipmap(GLuint texture, GLenum target, GLenum bindingTarget)
{
    TextureBinder binder(this, texture, target, bindingTarget);
    glGenerateMipmap(target);
}

void QOpenGLTextureHelper::qt_TextureStorage3D(GLuint texture, GLenum target, GLenum bindingTarget, GLsizei levels, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei depth)
{
    TextureBinder binder(this, texture, target, bindingTarget);
    glTexStorage3D(target, levels, internalFormat, width, height, depth);
}

void QOpenGLTextureHelper::qt_TextureStorage2D(GLuint texture, GLenum target, GLenum bindingTarget, GLsizei levels, GLenum internalFormat, GLsizei width, GLsizei height)
{
    TextureBinder binder(this, texture, target, bindingTarget);
    glTexStorage2D(target, levels, internalFormat, width, height);
}

void QOpenGLTextureHelper::qt_TextureStorage1D(GLuint texture, GLenum target, GLenum bindingTarget, GLsizei levels, GLenum internalFormat, GLsizei width)
{
    TextureBinder binder(this, texture, target, bindingTarget);
    glTexStorage1D(target, levels, internalFormat, width);
}

void QOpenGLTextureHelper::qt_TextureStorage3DMultisample(GLuint texture, GLenum target, GLenum bindingTarget, GLsizei samples, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedSampleLocations)
{
    TextureBinder binder(this, texture, target, bindingTarget);
    glTexStorage3DMultisample(target, samples, internalFormat, width, height, depth, fixedSampleLocations);
}

void QOpenGLTextureHelper::qt_TextureStorage2DMultisample(GLuint texture, GLenum target, GLenum bindingTarget, GLsizei samples, GLenum internalFormat, GLsizei width, GLsizei height, GLboolean fixedSampleLocations)
{
    TextureBinder binder(this, texture, target, bindingTarget);
    glTexStorage2DMultisample(target, samples, internalFormat, width, height, fixedSampleLocations);
}

void QOpenGLTextureHelper::qt_TextureImage3D(GLuint texture, GLenum target, GLenum bindingTarget, GLint level, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
    TextureBinder binder(this, texture, target, bindingTarget);
    glTexImage3D(target, level, internalFormat, width, height, depth, border, format, type, pixels);
}

void QOpenGLTextureHelper::qt_TextureImage2D(GLuint texture, GLenum target, GLenum bindingTarget, GLint level, GLenum internalFormat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
    TextureBinder binder(this, texture, target, bindingTarget);
    glTexImage2D(target, level, internalFormat, width, height, border, format, type, pixels);
}

void QOpenGLTextureHelper::qt_TextureImage1D(GLuint texture, GLenum target, GLenum bindingTarget, GLint level, GLenum internalFormat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
    TextureBinder binder(this, texture, target, bindingTarget);
    glTexImage1D(target, level, internalFormat, width, border, format, type, pixels);
}

void QOpenGLTextureHelper::qt_TextureSubImage3D(GLuint texture, GLenum target, GLenum bindingTarget, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels)
{
    TextureBinder binder(this, texture, target, bindingTarget);
    glTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
}

void QOpenGLTextureHelper::qt_TextureSubImage2D(GLuint texture, GLenum target, GLenum bindingTarget, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels)
{
    TextureBinder binder(this, texture, target, bindingTarget);
    glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
}

void QOpenGLTextureHelper::qt_TextureSubImage1D(GLuint texture, GLenum target, GLenum bindingTarget, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels)
{
    TextureBinder binder(this, texture, target, bindingTarget);
    glTexSubImage1D(target, level, xoffset, width, format, type, pixels);
}

void QOpenGLTextureHelper::qt_TextureImage3DMultisample(GLuint texture, GLenum target, GLenum bindingTarget, GLsizei samples, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedSampleLocations)
{
    TextureBinder binder(this, texture, target, bindingTarget);
    glTexImage3DMultisample(target, samples, internalFormat, width, height, depth, fixedSampleLocations);
}

void QOpenGLTextureHelper::qt_TextureImage2DMultisample(GLuint texture, GLenum target, GLenum bindingTarget, GLsizei samples, GLint internalFormat, GLsizei width, GLsizei height, GLboolean fixedSampleLocations)
{
    TextureBinder binder(this, texture, target, bindingTarget);
    glTexImage2DMultisample(target, samples, internalFormat, width, height, fixedSampleLocations);
}

void QOpenGLTextureHelper::qt_CompressedTextureSubImage1D(GLuint texture, GLenum target, GLenum bindingTarget, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *bits)
{
    TextureBinder binder(this, texture, target, bindingTarget);
    glCompressedTexSubImage1D(target, level, xoffset, width, format, imageSize, bits);
}

void QOpenGLTextureHelper::qt_CompressedTextureSubImage2D(GLuint texture, GLenum target, GLenum bindingTarget, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *bits)
{
    TextureBinder binder(this, texture, target, bindingTarget);
    glCompressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format, imageSize, bits);
}

void QOpenGLTextureHelper::qt_CompressedTextureSubImage3D(GLuint texture, GLenum target, GLenum bindingTarget, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *bits)
{
    TextureBinder binder(this, texture, target, bindingTarget);
    glCompressedTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, bits);
}

void QOpenGLTextureHelper::qt_CompressedTextureImage1D(GLuint texture, GLenum target, GLenum bindingTarget, GLint level, GLenum internalFormat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *bits)
{
    TextureBinder binder(this, texture, target, bindingTarget);
    glCompressedTexImage1D(target, level, internalFormat, width, border, imageSize, bits);
}

void QOpenGLTextureHelper::qt_CompressedTextureImage2D(GLuint texture, GLenum target, GLenum bindingTarget, GLint level, GLenum internalFormat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *bits)
{
    TextureBinder binder(this, texture, target, bindingTarget);
    glCompressedTexImage2D(target, level, internalFormat, width, height, border, imageSize, bits);
}

void QOpenGLTextureHelper::qt_CompressedTextureImage3D(GLuint texture, GLenum target, GLenum bindingTarget, GLint level, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *bits)
{
    TextureBinder binder(this, texture, target, bindingTarget);
    glCompressedTexImage3D(target, level, internalFormat, width, height, depth, border, imageSize, bits);
}

