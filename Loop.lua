local speed = 5

if ((window:keyMod(GL.MOD_SHIFT) and window:keyIsPressed(GL.KEY_UP))
    or (not window:keyMod(GL.MOD_SHIFT) and window:keyIsHeld(GL.KEY_UP)))
then
    camera:move( vec3.new(0, speed, 0) )
end
if ((window:keyMod(GL.MOD_SHIFT) and window:keyIsPressed(GL.KEY_DOWN))
    or (not window:keyMod(GL.MOD_SHIFT) and window:keyIsHeld(GL.KEY_DOWN)))
then
    camera:move( vec3.new(0, -speed, 0) )
end
if ((window:keyMod(GL.MOD_SHIFT) and window:keyIsPressed(GL.KEY_LEFT))
    or (not window:keyMod(GL.MOD_SHIFT) and window:keyIsHeld(GL.KEY_LEFT)))
then
    camera:move( vec3.new(-speed, 0, 0) )
end
if ((window:keyMod(GL.MOD_SHIFT) and window:keyIsPressed(GL.KEY_RIGHT))
    or (not window:keyMod(GL.MOD_SHIFT) and window:keyIsHeld(GL.KEY_RIGHT)))
then
    camera:move( vec3.new(speed, 0, 0) )
end

if (window:keyIsPressed(GL.KEY_SPACE))
then
    print("SPACE")
end
if (window:keyIsPressed(GL.KEY_SPACE, 2))
then
    print("DOUBLE SPACE")
end
if (window:keyIsPressed(GL.KEY_SPACE, 3))
then
    print("TRIPLE SPACE")
end

if (window:clickIsPressed(GL.LEFT_CLICK))
then
    print("LEFT CLICK")
end
if (window:clickIsPressed(GL.LEFT_CLICK, 2))
then
    print("DOUBLE LEFT CLICK")
end
if (window:clickIsPressed(GL.LEFT_CLICK, 3))
then
    print("TRIPLE LEFT CLICK")
end

if (plane:isHovered())
then
    --print("Hovered")
end
if (plane:isClicked())
then
    --print("Clicked")
end
if (plane:isHeld())
then
    --print("Held")
end


if (plane.alpha == 1)
then
    alpha_coeff = -0.003
elseif (plane.alpha == 0)
then
    alpha_coeff = 0.003
end

if (window:keyIsPressed(GL.KEY_C))
then
    print(window:getCursorPos())
end

if (window:keyIsHeld(GL.KEY_KP_ADD))
then
    camera:zoomIn(0.01)
end

if (window:keyIsHeld(GL.KEY_KP_SUBTRACT))
then
    camera:zoomOut(0.01)
end

if (window:keyIsPressed(GL.KEY_KP_DIVIDE))
then
    camera.fov = 150
    print(camera.fov)
    print(camera.zoom)
end

--plane.alpha = plane.alpha + alpha_coeff

local function isUpsideDown(plane)
    return plane.rotation.z > 90 or plane.rotation.z < -90
end

--print("is upside down:", plane_renderer:forEachB(isUpsideDown))