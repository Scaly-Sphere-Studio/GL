print("> Init.lua start")

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

local context = GL.Context.new(window)

camera = GL.Camera.create()
plane = GL.Plane.create()
texture = GL.Texture.create(window)
plane_renderer = GL.PlaneRenderer.create(window)

area = TR.Area.get(0)
area.w = 300
area.h = 300
local fmt = area:getFmt()
fmt.charsize = 50
fmt.text_color.func = TR.ColorFunc.Rainbow
area:setFmt(fmt)
area.string = "Lorem ipsum dolor sit amet."

texture.text_area_id = area.id
texture.type = GL.TextureType.Text

camera.position = vec3.new(0, 0, 3)
camera.proj_type = GL.Projection.OrthoFixed

plane_renderer.title = "Random title"
local chunk = GL.Chunk.new(camera)

for i = 0, 16, 1
do
  for j = 0, 16, 1
  do
    local p = GL.Plane.create(window)
    p.texture_id = texture.id
    p:scale(vec3.new(20));
    p:translate(vec3.new(-150.0 + 20.0 * (j % 16), -150.0 + (i % 16) * 20.0, 0.0));
    chunk.planes:add(p)
  end
end
plane_renderer.chunks:add(chunk)

plane:scale(vec3.new(300))
plane.texture_id = texture.id
plane.passive_func_id = 1
plane.on_click_func_id = 1
plane.hitbox = GL.PlaneHitbox.Full

chunk.planes:clear()
chunk.planes:add(plane)
plane_renderer.chunks:add(chunk)

print("> Init.lua end")