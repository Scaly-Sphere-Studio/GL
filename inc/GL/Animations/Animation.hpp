#pragma once

#include <GL/Objects/Basic.hpp>
#include <SSS/Math.hpp>
#include <glm/gtc/quaternion.hpp>
#include <map>

using namespace std::chrono_literals;

SSS_BEGIN

// Time manager
class SSS_GL_API Track : public SSS::Subject, public SSS::_EventRegistry<Track>
{

public:
    friend SSS::_EventRegistry<Track>;

    enum class LectureMode
    {
        Once,
        Reverse,
        Loop,
        PingPong
    };

    using EaseType  = SSS::Math::EaseType;
    using TimePoint = std::chrono::steady_clock::time_point;
    using Duration  = std::chrono::duration<float>;

    Track() = default;
    //Track(const Track&) = default;
    virtual ~Track() = default;


    void setLectureMode(LectureMode mode) { _mode = mode; }
    void setEase(EaseType ef) { _ease = ef; }

    // Controls
    void start();
    void play();
    void stop();
    void pause();
    void resume();
    void restart();

    // Getters
    bool isRunning() const { return _playing; }
    bool isPaused()  const { return _paused; }
    std::chrono::duration<double> getDuration() { return _duration; };
    double getDurationSeconds() const { return std::chrono::duration_cast<std::chrono::milliseconds>(_duration).count() / 1000.; };

    const double getCurrentTime() const { return std::chrono::duration_cast<std::chrono::milliseconds>(_currentTime).count() / 1000.;};
    double getNormalizedTime() { return normalizedTime(); };


    // Setters
    void setDuration(float seconds) { _duration = std::chrono::duration<float>(seconds); if (_duration == 0s) stop(); }
    void setSpeed(float sp) { _speed = sp; if (_speed == 0) pause();  }
    double duration() const { return std::chrono::duration_cast<std::chrono::milliseconds>(_duration).count() /1000.; }

    // Call every frame
    void update();

protected:

    double normalizedTime()
    {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - _startTime) - _pauseAccum;
        double raw = 0.;

        switch (_mode)
        {
        case LectureMode::Once:
        case LectureMode::Loop:
            raw = (_currentTime / _duration) * _speed;
            break;
        case LectureMode::Reverse:
            raw = 1.0 - (_currentTime / _duration) * _speed;
            break;
        case LectureMode::PingPong:
            auto cycleTime = _duration * 2.0; // Full ping-pong cycle
            if (_currentTime > cycleTime)
            {
                _currentTime = _currentTime - cycleTime;
            }

            if (elapsed > _duration)
            {
                auto newTime = std::fmod(_currentTime.count(), cycleTime.count());
                raw = std::chrono::duration<double>(newTime) / _duration;
            }
            else{ raw = (_currentTime / _duration) * _speed; }
            break;
        }

        return std::clamp(raw, 0., 1.);
    }


protected:
    float _speed        = 1.0f;

    std::chrono::duration<double> _duration     = 0s;
    std::chrono::duration<double> _currentTime  = 0s;
    std::chrono::duration<double> _pauseAccum   = 0s;

    bool _playing   = false;
    bool _paused    = false;

    LectureMode  _mode = LectureMode::Once;
    EaseType     _ease = EaseType::Linear;

    //bool m_forward = true; // reserved for future manual stepping


    std::chrono::steady_clock::time_point _startTime{};
    std::chrono::steady_clock::time_point _pauseStart{};

private:
    static void _register();
};


template<typename T>
class SSS_GL_API PropertyAnimator : public Track {
public:
    using Callback = std::function<void(const T&)>;

    PropertyAnimator() = default;

    void addKeyframe(float time, const T& value, EaseType easing = EaseType::Linear) {
        SSS::Math::GradientPoint<T> kf{ time, value, easing };
        track.push(kf);
        setDuration(std::max((float)getDurationSeconds(), track.getDuration()));
    }

    void addKeyframe(float time, const T& value, SSS::Math::EasingFunction customEasing) {
        SSS::Math::GradientPoint<T> kf{ time, value, customEasing };
        track.push(kf);
        setDuration(std::max((float)getDurationSeconds(), track.getDuration()));
    }

    void setCallback(Callback cb) {
        callback = cb;
    }

    void update() {
        Track::update();

        if (callback) {
            T value = track.evaluate(getCurrentTime());
            callback(value);
        }
    }

    T getCurrentValue() const {
        return track.evaluate(getCurrentTime());
    }

protected:
    SSS::Math::Gradient<T> track;
    Callback callback;
};

// ============================================================================
// Transform Animator (animates position, rotation, scale together)
// ============================================================================

class SSS_GL_API TransformAnimator : public Track {
    using EaseType = SSS::Math::EaseType;
    using EasingFunction = SSS::Math::EasingFunction;
public:
    struct Transform {
        glm::vec3 position{ 0.0f };
        glm::quat rotation{ 1.0f, 0.0f, 0.0f, 0.0f };
        glm::vec3 scale{ 1.0f };
    };

    using Callback = std::function<void(const Transform&)>;

    void addPositionKeyframe(float time, const glm::vec3& pos, EaseType easing = EaseType::Linear) {
        SSS::Math::GradientPoint<glm::vec3> p{time, pos, easing};
        positionTrack.push(p);
        updateDuration();
    }

    void addPositionKeyframe(float time, const glm::vec3& pos, EasingFunction customEasing) {
        SSS::Math::GradientPoint<glm::vec3> p{ time, pos, customEasing };
        positionTrack.push(p);
        updateDuration();
    }

    void addRotationKeyframe(float time, const glm::quat& rot, EaseType easing = EaseType::Linear) {
        SSS::Math::GradientPoint<glm::quat> p{ time, rot, easing };
        rotationTrack.push(p);
        updateDuration();
    }

    void addRotationKeyframe(float time, const glm::quat& rot, EasingFunction customEasing) {
        SSS::Math::GradientPoint<glm::quat> p{ time, rot, customEasing };
        rotationTrack.push(p);
        updateDuration();
    }

    void addScaleKeyframe(float time, const glm::vec3& scale, EaseType easing = EaseType::Linear) {
        SSS::Math::GradientPoint<glm::vec3> p{ time, scale, easing };
        scaleTrack.push(p);
        updateDuration();
    }

    void addScaleKeyframe(float time, const glm::vec3& scale, EasingFunction customEasing) {
        SSS::Math::GradientPoint<glm::vec3> p{ time, scale, customEasing };
        scaleTrack.push(p);
        updateDuration();
    }

    void setCallback(Callback cb) {
        callback = cb;
    }

    void update() {
        Track::update();

        if (callback) {
            Transform transform;
            transform.position = positionTrack.evaluate(getCurrentTime());
            transform.rotation = rotationTrack.evaluate(getCurrentTime());
            transform.scale = scaleTrack.evaluate(getCurrentTime());
            callback(transform);
        }
    }

    Transform getCurrentTransform() const {
        Transform transform;
        transform.position = positionTrack.evaluate(getCurrentTime());
        transform.rotation = rotationTrack.evaluate(getCurrentTime());
        transform.scale = scaleTrack.evaluate(getCurrentTime());
        return transform;
    }

private:
    SSS::Math::Gradient<glm::vec3> positionTrack;
    SSS::Math::Gradient<glm::quat> rotationTrack;
    SSS::Math::Gradient<glm::vec3> scaleTrack;
    Callback callback;

    void updateDuration() {
        float maxDuration = std::max({
            positionTrack.getDuration(),
            rotationTrack.getDuration(),
            scaleTrack.getDuration()
            });
        setDuration(maxDuration);
    }
};

// ============================================================================
// Animation Timeline (manages multiple animations)
// ============================================================================

class SSS_GL_API Timeline {
public:
    void addClip(const std::string& name, std::shared_ptr<Track> track) {
        tracks[name] = track;
    }

    void play(const std::string& name) {
        if (auto it = tracks.find(name); it != tracks.end()) {
            it->second->play();
        }
    }

    void playAll() {
        for (auto& [name, clip] : tracks) {
            clip->play();
        }
    }

    void pause(const std::string& name) {
        if (auto it = tracks.find(name); it != tracks.end()) {
            it->second->pause();
        }
    }

    void pauseAll() {
        for (auto& [name, clip] : tracks) {
            clip->pause();
        }
    }

    void stop(const std::string& name) {
        if (auto it = tracks.find(name); it != tracks.end()) {
            it->second->stop();
        }
    }

    void stopAll() {
        for (auto& [name, clip] : tracks) {
            clip->stop();
        }
    }

    void update() {
        for (auto& [name, clip] : tracks) {
            clip->update();
        }
    }

    std::shared_ptr<Track> getClip(const std::string& name) {
        auto it = tracks.find(name);
        return it != tracks.end() ? it->second : nullptr;
    }

private:
    std::map<std::string, std::shared_ptr<Track>> tracks;
};

SSS_END;
