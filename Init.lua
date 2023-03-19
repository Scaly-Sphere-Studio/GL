print("> Init.lua start")

-- Window
do
    local args = GL.WindowArgs.new()
    args.w = 1280
    args.h = 720
    args.title = "SSS/GL - Demo Window"
    args.fullscreen = false
    args.maximized = false
    args.iconified = false
    args.hidden = false

    window = GL.Window.create(args)
    window.vsync = true
end
print("  > Window created")

local context = GL.Context.new(window)

-- Text area
do
    area = TR.Area.create()
    area.w = 300
    area.h = 300
    area.focusable = true
    local fmt = area:getFmt()
    fmt.charsize = 50
    fmt.text_color.func = TR.ColorFunc.Rainbow
    area:setFmt(fmt)
    area.string = "Lorem ipsum dolor sit amet."
end
print("  > TR::Area created")

-- Window objects
do
    camera = GL.Camera.create()
    texture = GL.Texture.create(area)
    plane = GL.Plane.create(texture)

    plane_renderer = GL.PlaneRenderer.create(camera)
    window:addRenderer(plane_renderer)

    line_shaders = GL.Shaders.create("glsl/line.vert", "glsl/line.frag")
    line_renderer = GL.LineRenderer.create()
    window:addRenderer(line_renderer)
end
print("  > Window objects created")

-- Settings
do
    camera.position = vec3.new(0, 0, 3)
    camera.proj_type = GL.Projection.OrthoFixed
    
    plane:scale(300)
    plane.hitbox = GL.PlaneHitbox.Full

    plane_renderer.planes:add(plane)
    
    line_renderer.shaders = line_shaders
    line_renderer.camera = camera
end

print("> Init.lua end")