  #version 330 core
  layout (location = 0) in vec3 aPos;
  layout (location = 1) in vec4 aColor;
  layout (location = 2) in float aSize;

  out vec4 fragColor;

  uniform mat4 projection;
  uniform mat4 view;

  void main()
  {
      gl_Position = projection * view * vec4(aPos, 1.0);
      gl_PointSize = aSize * 120.0;
      fragColor = aColor;
  }
