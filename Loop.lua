local speed = 1.5

if (GL.keyIsHeld(GL.KEY_UP))
then
    camera:move( vec3.new(0, speed, 0) )
end
if (GL.keyIsHeld(GL.KEY_DOWN))
then
    camera:move( vec3.new(0, -speed, 0) )
end
if (GL.keyIsHeld(GL.KEY_LEFT))
then
    camera:move( vec3.new(-speed, 0, 0) )
end
if (GL.keyIsHeld(GL.KEY_RIGHT))
then
    camera:move( vec3.new(speed, 0, 0) )
end

if (GL.keyIsPressed(GL.KEY_SPACE))
then
    print("SPACE")
end
if (GL.keyIsPressed(GL.KEY_SPACE, 2))
then
    print("DOUBLE SPACE")
end
if (GL.keyIsPressed(GL.KEY_SPACE, 3))
then
    print("TRIPLE SPACE")
end

if (GL.clickIsPressed(GL.LEFT_CLICK))
then
    print("LEFT CLICK")
end
if (GL.clickIsPressed(GL.LEFT_CLICK, 2))
then
    print("DOUBLE LEFT CLICK")
end
if (GL.clickIsPressed(GL.LEFT_CLICK, 3))
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
    alpha_coeff = -0.01
elseif (plane.alpha == 0)
then
    alpha_coeff = 0.01
end

if (GL.keyIsPressed(GL.KEY_C))
then
    print(GL.getCursorPos())
end

if (GL.keyIsPressed(GL.KEY_ENTER))
then
    GL.close()
    return
end

if (plane:isHeld())
then
    local x, y = GL.getCursorDiff()
    if (x ~= 0 or y ~= 0)
    then
        plane:translate(vec3.new(x, y, 0))
    end
end

plane.alpha = plane.alpha + alpha_coeff