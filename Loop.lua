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