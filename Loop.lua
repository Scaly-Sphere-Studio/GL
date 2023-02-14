local speed = 1.5

if (window:keyIsHeld(GL.KEY_UP))
then
    camera:move( vec3.new(0, speed, 0) )
end
if(window:keyIsHeld(GL.KEY_DOWN))
then
    camera:move( vec3.new(0, -speed, 0) )
end
if(window:keyIsHeld(GL.KEY_LEFT))
then
    camera:move( vec3.new(-speed, 0, 0) )
end
if(window:keyIsHeld(GL.KEY_RIGHT))
then
    camera:move( vec3.new(speed, 0, 0) )
end

if(window:keyIsPressed(GL.KEY_SPACE))
then
    print("SPACE")
end
if(window:keyIsPressed(GL.KEY_SPACE, 2))
then
    print("DOUBLE SPACE")
end
if(window:keyIsPressed(GL.KEY_SPACE, 3))
then
    print("TRIPLE SPACE")
end

if(window:clickIsPressed(GL.LEFT_CLICK))
then
    print("LEFT CLICK")
end
if(window:clickIsPressed(GL.LEFT_CLICK, 2))
then
    print("DOUBLE LEFT CLICK")
end
if(window:clickIsPressed(GL.LEFT_CLICK, 3))
then
    print("TRIPLE LEFT CLICK")
end

if (plane.alpha == 1)
then
    alpha_coeff = -0.01
elseif (plane.alpha == 0)
then
    alpha_coeff = 0.01
end

plane.alpha = plane.alpha + alpha_coeff