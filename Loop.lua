local speed = 1.5

if (window:keyIsPressed(GL.KEY_UP))
then
  camera:move( vec3.new(0, speed, 0) )
end
if(window:keyIsPressed(GL.KEY_DOWN))
then
  camera:move( vec3.new(0, -speed, 0) )
end
if(window:keyIsPressed(GL.KEY_LEFT))
then
  camera:move( vec3.new(-speed, 0, 0) )
end
if(window:keyIsPressed(GL.KEY_RIGHT))
then
  camera:move( vec3.new(speed, 0, 0) )
end

if (plane.alpha == 1)
then
  alpha_coeff = -0.01
elseif (plane.alpha == 0)
then
  alpha_coeff = 0.01
end

plane.alpha = plane.alpha + alpha_coeff