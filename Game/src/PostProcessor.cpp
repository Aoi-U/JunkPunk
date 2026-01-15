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
  : vbo(vertices)
{
  vao.Bind();
  vbo.Bind();

	// Links VBO attributes such as coordinates and colors to VAO
  vao.LinkAttributes(vbo, 0, 2, GL_FLOAT, sizeof(PostProcessVertex), (void*)0); // position
  vao.LinkAttributes(vbo, 1, 2, GL_FLOAT, sizeof(PostProcessVertex), (void*)(2 * sizeof(float))); // texCoord
  vao.Unbind();
	vbo.Unbind();

  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  // create a color attachment texture
  texColorbuffer;
  glGenTextures(1, &texColorbuffer);
  glBindTexture(GL_TEXTURE_2D, texColorbuffer);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColorbuffer, 0);

  // create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
  glGenRenderbuffers(1, &rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height); // use a single renderbuffer object for both a depth AND stencil buffer.
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // now actually attach it
  // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PostProcessor::BindVAO()
{
  vao.Bind();
}

void PostProcessor::BindTexture()
{
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texColorbuffer);
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

void PostProcessor::Resize(GLuint width, GLuint height)
{
  // resize texture
  glBindTexture(GL_TEXTURE_2D, texColorbuffer);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
  // resize renderbuffer
  glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
}

void PostProcessor::Cleanup()
{
  glDeleteFramebuffers(1, &fbo);
  glDeleteTextures(1, &texColorbuffer);
  glDeleteRenderbuffers(1, &rbo);
  vao.Delete();
  vbo.Delete();
} 


