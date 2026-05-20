#ifndef PENGO_STATE_H
#define PENGO_STATE_H

#include <memory>

namespace dae
{
    class PengoCharacter;

    class PengoState
    {
    public:
        virtual ~PengoState() = default;
        virtual void OnEnter(PengoCharacter *pPengo) = 0;
        virtual void OnExit(PengoCharacter *pPengo) = 0;
        virtual std::unique_ptr<PengoState> HandleInput(PengoCharacter *pPengo) = 0;
        virtual std::unique_ptr<PengoState> Update(PengoCharacter *pPengo) = 0;
    };

    class IdleState final : public PengoState
    {
    public:
        void OnEnter(PengoCharacter *pPengo) override;
        void OnExit(PengoCharacter *pPengo) override;
        std::unique_ptr<PengoState> HandleInput(PengoCharacter *pPengo) override;
        std::unique_ptr<PengoState> Update(PengoCharacter *pPengo) override;
    };

    class MovingState final : public PengoState
    {
    public:
        void OnEnter(PengoCharacter *pPengo) override;
        void OnExit(PengoCharacter *pPengo) override;
        std::unique_ptr<PengoState> HandleInput(PengoCharacter *pPengo) override;
        std::unique_ptr<PengoState> Update(PengoCharacter *pPengo) override;

    private:
        void UpdateAnimation(PengoCharacter *pPengo);
        float m_animationTimer{0.0f};
        int m_currentFrame{0};
    };

    class PushingState final : public PengoState
    {
    public:
        void OnEnter(PengoCharacter *pPengo) override;
        void OnExit(PengoCharacter *pPengo) override;
        std::unique_ptr<PengoState> HandleInput(PengoCharacter *pPengo) override;
        std::unique_ptr<PengoState> Update(PengoCharacter *pPengo) override;

    private:
        float m_timer{0.0f};
        static constexpr float PUSH_DURATION = 0.4f;
    };

    class DyingState final : public PengoState
    {
    public:
        void OnEnter(PengoCharacter *pPengo) override;
        void OnExit(PengoCharacter *pPengo) override;
        std::unique_ptr<PengoState> HandleInput(PengoCharacter *pPengo) override;
        std::unique_ptr<PengoState> Update(PengoCharacter *pPengo) override;
    };
}

#endif // PENGO_STATE_H