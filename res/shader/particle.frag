  #version 330 core
  in vec4 fragColor;
  out vec4 FragColor;

  void main()
  {
      vec2 coord = gl_PointCoord - vec2(0.5);
      float dist = length(coord);
      if (dist > 0.5)
          discard;
      float alpha = fragColor.a * (1.0 - smoothstep(0.3, 0.5, dist));
      FragColor = vec4(fragColor.rgb, alpha);
  }