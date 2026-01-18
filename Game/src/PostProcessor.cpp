#include "PostProcessor.h"

std::vector<PostProcessVertex> vertices{ // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
  // positions   // texCoords
  PostProcessVertex{{-1.0f, 1.0f}, {0.0f, 1.0f}},
  PostProcessVertex{{-1.0f, -1.0f}, {0.0f, 0.0f}},
  PostProcessVertex{{1.0f, -1.0f}, {1.0f, 0.0f}},
  PostProcessVertex{{-1.0f, 1.0f}, {0.0f, 1.0f}},
  PostProcessVertex{{1.0f, -1.0f}, {1.0f, 0.0f}},
  PostProcessVertex{{1.0f, 1.0f}, {1.0f, 1.0f}}
};

PostProcessor::PostProcessor(GLuint width, GLuint height)
	: vbo(vertices), width(width), height(height)
{
  vao.Bind();
  vbo.Bind();

	// Links VBO attributes such as coordinates and colors to VAO
  vao.LinkAttributes(vbo, 0, 2, GL_FLOAT, sizeof(PostProcessVertex), (void*)0); // position
  vao.LinkAttributes(vbo, 1, 2, GL_FLOAT, sizeof(PostProcessVertex), (void*)(2 * sizeof(float))); // texCoord
  vao.Unbind();
	vbo.Unbind();

  // configure MSAA framebuffer
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  // create a color attachment texture
  glGenTextures(1, &texColorbufferMultisampled);
  glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texColorbufferMultisampled);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGB, width, height, GL_TRUE);
  glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, texColorbufferMultisampled, 0);

  // create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
  glGenRenderbuffers(1, &rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, width, height); 
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); 

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // configure second post processing framebuffer
  glGenFramebuffers(1, &intermediateFbo);
  glBindFramebuffer(GL_FRAMEBUFFER, intermediateFbo);
  
  // create color attachment texture
  glGenTextures(1, &screenTexture);
  glBindTexture(GL_TEXTURE_2D, screenTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screenTexture, 0);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    std::cout << "ERROR::FRAMEBUFFER:: Intermediate framebuffer is not complete!" << std::endl;
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PostProcessor::BindVAO()
{
  vao.Bind();
}

void PostProcessor::BindTexture()
{
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, screenTexture);
}

void PostProcessor::BindFBO()
{
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glEnable(GL_DEPTH_TEST);
}

void PostProcessor::Unbind()
{
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glDisable(GL_DEPTH_TEST);
}

void PostProcessor::Blit()
{
  glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, intermediateFbo);
  glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

void PostProcessor::Resize(GLuint width, GLuint height)
{
  // resize texture
  glBindTexture(GL_TEXTURE_2D, texColorbufferMultisampled);

  //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGB, width, height, GL_TRUE);

  // resize renderbuffer
  glBindRenderbuffer(GL_RENDERBUFFER, rbo);

	//glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
  glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, width, height); 

	// resize screen texture
  glBindTexture(GL_TEXTURE_2D, screenTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

}

void PostProcessor::Cleanup()
{
  glDeleteFramebuffers(1, &fbo);
  glDeleteTextures(1, &texColorbufferMultisampled);
  glDeleteRenderbuffers(1, &rbo);
  vao.Delete();
  vbo.Delete();
} 


