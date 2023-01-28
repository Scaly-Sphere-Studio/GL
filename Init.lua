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

local context = GL.Context.new(window)

-- Text area
do
    area = TR.Area.create()
    area.w = 300
    area.h = 300
    local fmt = area:getFmt()
    fmt.charsize = 50
    fmt.text_color.func = TR.ColorFunc.Rainbow
    area:setFmt(fmt)
    area.string = "Lorem ipsum dolor sit amet."
end

-- Window objects
do
    plane_renderer = GL.PlaneRenderer.create()
    texture = GL.Texture.create(area)

    line_shaders = GL.Shaders.create("glsl/line.vert", "glsl/line.frag")
    line_renderer = GL.LineRenderer.create()

    window:setRenderers({ plane_renderer, line_renderer })

    camera = GL.Camera.create()
    plane = GL.Plane.create(texture)
end

-- Settings
do
    plane_renderer.title = "Basic plane renderer"
    local chunk = GL.Chunk.new(camera)
    chunk.planes:add(plane)
    plane_renderer.chunks:add(chunk)
    
    line_renderer.title = "Basic line renderer"
    line_renderer.shaders = line_shaders
    line_renderer.camera = camera
    
    camera.position = vec3.new(0, 0, 3)
    camera.proj_type = GL.Projection.OrthoFixed
    
    plane:scale(300)
    plane.passive_func_id = 1
    plane.on_click_func_id = 1
    plane.hitbox = GL.PlaneHitbox.Full
end

print("> Init.lua end")