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

    window = GL.Window.new(args)
    window.vsync = true
end
print("  > Window created")

-- Text area
do
    area = TR.Area.new()
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
    camera = GL.Camera.new()
    plane = GL.Plane.new(area)

    plane_renderer = GL.PlaneRenderer.new(camera)
    window:addRenderer(plane_renderer)

    line_shaders = GL.Shaders.new("glsl/line.vert", "glsl/line.frag")
    line_renderer = GL.LineRenderer.new()
    window:addRenderer(line_renderer)
end
print("  > Window objects created")

-- Settings
do
    camera.position = vec3.new(0, 0, 300)
    camera.z_far = 1000
    camera.proj_type = GL.Projection.OrthoFixed
    
    plane:rotate(vec3.new(0, 45, 45))
    plane:scale(300)
    plane.hitbox = GL.PlaneHitbox.Full

    plane_renderer.planes:add(plane)
    
    --plane_renderer:forEach(function(p)
    --    p:setTextureCallback(function(p)
    --        p:rotate(vec3.new(0, 0, 1))
    --    end)
    --end)

    line_renderer.shaders = line_shaders
    line_renderer.camera = camera
end

print("> Init.lua end")