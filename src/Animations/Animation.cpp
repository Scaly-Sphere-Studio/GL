#include <GL/Animations/Animation.hpp>

SSS_BEGIN

void Track::_register()
{
    REGISTER_EVENT("SSS_TRACK_STOP");
    REGISTER_EVENT("SSS_TRACK_PLAY");
    REGISTER_EVENT("SSS_TRACK_PAUSE");
    REGISTER_EVENT("SSS_TRACK_RESUME");
    REGISTER_EVENT("SSS_TRACK_LOOPED");
}

void Track::start() 
{
    _playing       = true;
    _paused        = false;
    _startTime     = std::chrono::steady_clock::now();
    _pauseAccum    = 0s;

    EMIT_EVENT("SSS_TRACK_PLAY");
}

void Track::play()
{
    start();
}

void Track::stop()
{
    _playing        = false;
    _paused         = false;
    _currentTime    = 0s;

    EMIT_EVENT("SSS_TRACK_STOP");
}

void Track::pause()
{
    if (!_playing || _paused) return;
    _paused = true;
    _pauseStart = std::chrono::steady_clock::now();
    EMIT_EVENT("SSS_TRACK_PAUSE");
}

void Track::restart()
{
    stop();
    play();
}

void Track::resume()
{
    if (!_playing || !_paused) return;
    _paused = false;
    _pauseAccum += std::chrono::steady_clock::now() - _pauseStart;
    EMIT_EVENT("SSS_TRACK_RESUME");
}

void Track::update()
{
    if (!_playing) return;

    auto now = std::chrono::steady_clock::now();
    auto elapsed = now - _startTime - _pauseAccum;

    _currentTime = elapsed * _speed;

    switch (_mode) 
    {
    case LectureMode::Once: 
        {
            if (_currentTime >= _duration) {
                _currentTime = _duration;
                stop();
            }
        }
        return;
    case LectureMode::Loop: 
        {
            if (_currentTime >= _duration) {
                _currentTime = std::chrono::duration<double>(std::fmod(_currentTime.count(), _duration.count()));
                _startTime = now - std::chrono::duration_cast<std::chrono::nanoseconds>(_currentTime / _speed);
                REGISTER_EVENT("SSS_TRACK_LOOPED");
            }
        }
        return;
    case LectureMode::PingPong:
        {
            auto cycleTime = _duration * 2.0; // Full ping-pong cycle
            if (_currentTime > cycleTime) { restart(); return; }

            auto newTime = std::fmod(_currentTime.count(), cycleTime.count());
            auto cur =  _currentTime.count();

            if (elapsed > _duration) 
            {
                // Reverse phase
                _currentTime = _duration - (std::chrono::duration<double>(newTime) - _duration);
            }

        }
        return;
    case LectureMode::Reverse: 
        {
            _currentTime = _duration - (elapsed * _speed);
            if (_currentTime < 0s) { stop(); }
        }
        return;
    default :
        return;
    }
}

SSS_END;