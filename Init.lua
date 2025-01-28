print("> Init.lua start")

-- Window
do
    local args = GL.WindowArgs.new()
    args.w = 1280
    args.h = 720
    args.title = "SSS/GL - Demo Window"
    --args.monitor_id = 1
    args.fullscreen = false
    args.maximized = false
    args.iconified = false
    args.hidden = false

    window = GL.Window.new(args)
    window.vsync = true
    --window.fps_limit = 20
end
print("  > Window created")

-- Text area
--do
--    TR.addFontDir("C:/dev/fonts")
--    area = TR.Area.new(1080, 720)
--    area.print_mode = TR.PrintMode.Typewriter
--    area.focusable = true
--    area.TW_speed = 34
--    area:setMargins(20, 20)
--    local fmt = area:getFmt()
--    fmt.font = "lmroman12-regular.otf"
--    fmt.charsize = 34
--    fmt.line_spacing = 2
--    fmt.has_outline = true
--    fmt.outline_size = 3
--    fmt.outline_color.rgb = 0x202020
--    fmt.alignment = TR.Alignment.Center
--    area:setFmt(fmt)
--    area.string = [[Afficher du texte peut sembler assez simple de prime abord, mais dès lors que l'on décide de s'aventurer un peu plus loin que l'alphabet latin, on découvre une multitude de facettes plus complexes les unes que les autres. Fort heureusement, grace à certaines bibliothèques logicielles libres, notamment FreeType et HarfBuzz, nous pouvons nous focaliser sur les dernières étapes du processus. Cela ouvre la porte à toutes sortes de polices et d'alphabets différents, (ce qui va souvent de paire).{{
--    "font":"YUGOTHM.ttc",
--    "alignment": "Center",
--    "lng_tag": "ja",
--    "lng_script": "Jpan",
--    "tw_short_pauses": "、",
--    "line_spacing": 3
--}}
--このデモが気に入ったら、私を雇ってください{{}}]]
--end
--print("  > TR::Area created")

do
    local demo_str = 
[[{{"effect":"Waves", "effect_offset": 50}}Versatile,{{}} {{"effect":"Vibrate", "effect_offset": 1, "font": "CALIBRIB.TTF", "outline_size": 2}}robust,{{}} and {{"font": "SEGOEPR.TTF", "has_shadow": true}}optimised
{{}}{{"font": "INKFREE.TTF", "outline_size": 2, "charsize": 60}} Text Rendering {{}}
for video games and applications!]]

    TR.addFontDir("C:/dev/fonts")

    TR.default_fmt.font = "CALIBRI.TTF"
    TR.default_fmt.charsize = 30
    TR.default_fmt.has_outline = true
    --TR.default_fmt.has_shadow = true
    TR.default_fmt.outline_size = 1
    TR.default_fmt.line_spacing = 1.75
    TR.default_fmt.alignment = TR.Alignment.Center
    --TR.default_fmt.effect = TR.Effect.Waves
    --TR.default_fmt.effect_offset = 50
    --TR.default_fmt.tw_short_pauses = ""

    area = TR.Area.new()

    area:setMargins(30, 30)
    area.string = demo_str
    area.clear_color = RGBA.new(0xFF888888)
    area.wrapping = true
    --area.print_mode = TR.PrintMode.Typewriter
    area.TW_speed = 20
    --area.focusable = true;
    --area.focus = true;
end


-- Window objects
do
    camera = GL.Camera.new()
    plane = GL.Plane.new(area)

    plane_renderer = GL.PlaneRenderer.new(camera)
    line_renderer = GL.LineRenderer.new()

    window.renderers = { line_renderer, plane_renderer }
end
print("  > Window objects created")

-- Settings
do
    camera.position = vec3.new(0, 0, 3)
    camera.proj_type = GL.Projection.OrthoFixed
    
    plane:scale(area.h)
    plane:translate(vec3.new(0, 0, 1))
    plane.hitbox = GL.PlaneHitbox.Full

    plane_renderer.planes:add(plane)
    
    --plane_renderer:forEach(function(p)
    --    p:setTextureCallback(function(p)
    --        p:rotate(vec3.new(0, 0, 1))
    --    end)
    --end)

    line_renderer.camera = camera
end

print("> Init.lua end")