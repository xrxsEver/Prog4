#ifndef PENGO_STATE_H
#define PENGO_STATE_H

namespace dae
{
    class PengoCharacter;

    class PengoState
    {
    public:
        virtual ~PengoState() = default;
        virtual void OnEnter(PengoCharacter *pPengo) = 0;
        virtual void OnExit(PengoCharacter *pPengo) = 0;
        virtual PengoState *HandleInput(PengoCharacter *pPengo) = 0;
        virtual PengoState *Update(PengoCharacter *pPengo) = 0;
    };

    class IdleState final : public PengoState
    {
    public:
        void OnEnter(PengoCharacter *pPengo) override;
        void OnExit(PengoCharacter *pPengo) override;
        PengoState *HandleInput(PengoCharacter *pPengo) override;
        PengoState *Update(PengoCharacter *pPengo) override;
    };

    class MovingState final : public PengoState
    {
    public:
        void OnEnter(PengoCharacter *pPengo) override;
        void OnExit(PengoCharacter *pPengo) override;
        PengoState *HandleInput(PengoCharacter *pPengo) override;
        PengoState *Update(PengoCharacter *pPengo) override;

    private:
        float m_animationTimer{0.0f};
        float m_stateTimer{0.0f};
        int m_currentFrame{0};
    };

    class PushingState final : public PengoState
    {
    public:
        void OnEnter(PengoCharacter *pPengo) override;
        void OnExit(PengoCharacter *pPengo) override;
        PengoState *HandleInput(PengoCharacter *pPengo) override;
        PengoState *Update(PengoCharacter *pPengo) override;
    };

    class DyingState final : public PengoState
    {
    public:
        void OnEnter(PengoCharacter *pPengo) override;
        void OnExit(PengoCharacter *pPengo) override;
        PengoState *HandleInput(PengoCharacter *pPengo) override;
        PengoState *Update(PengoCharacter *pPengo) override;
    };
}

#endif // PENGO_STATE_H