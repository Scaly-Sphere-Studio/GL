print("> Demo.lua start")

-- Create Window
do
  local args = GL.WindowArgs.new()
  args.w = 1280
  args.h = 720
  args.title = "SSS/GL - Demo Window"
  args.fullscreen = false
  args.maximized = true
  args.iconified = false
  args.hidden = false
  
  window = GL.Window.create(args)
  window.vsync = true
end

print("> Demo.lua end")