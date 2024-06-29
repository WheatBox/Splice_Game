#include "Utility.h"

#include <FrameUtility/UTF8Utils.h>
#include <FrameRender/Renderer.h>
#include <FrameAsset/Sprite.h>

#include "Components/Editor/EditorDeviceComponent.h"
#include "Components/Machine/DeviceComponent.h"

const UnicodeString & GetKeyName(Frame::EKeyId keyId) {
#define __FORMULA(_keyId, _name) { Frame::EKeyId::_keyId, Frame::UTF8Utils::ToUnicode(_name) }
#define F(_keyId_and_name) __FORMULA(eKI_##_keyId_and_name, #_keyId_and_name)

    static const std::unordered_map<Frame::EKeyId, UnicodeString> map {
        F(Space),
        __FORMULA(eKI_Quote, "'"),
        __FORMULA(eKI_Comma, ","),
        __FORMULA(eKI_Subtract, "-"),
        __FORMULA(eKI_Period, "."),
        __FORMULA(eKI_Slash, "/"),

        F(0), F(1), F(2), F(3), F(4),
        F(5), F(6), F(7), F(8), F(9),

        __FORMULA(eKI_Semicolon, ";"),
        __FORMULA(eKI_Equal, "="),

        F(A), F(B), F(C), F(D), F(E),
        F(F), F(G), F(H), F(I), F(J),
        F(K), F(L), F(M), F(N), F(O),
        F(P), F(Q), F(R), F(S), F(T),
        F(U), F(V), F(W), F(X), F(Y),
        F(Z),

        __FORMULA(eKI_LeftBracket, "["),
        __FORMULA(eKI_BackSlash, "\\"),
        __FORMULA(eKI_RightBracket, "]"),
        __FORMULA(eKI_BackQuote, "`"),

        F(World1_161),
        F(World2_162),

        __FORMULA(eKI_Escape, "Esc"),
        F(Enter),
        F(Tab),
        F(Backspace),
        F(Insert),
        F(Delete),
        F(Right), F(Left), F(Down), F(Up),
        __FORMULA(eKI_PageUp, "Page Up"),
        __FORMULA(eKI_PageDown, "Page Down"),
        F(Home),
        F(End),
        __FORMULA(eKI_CapsLock, "Caps Lock"),
        __FORMULA(eKI_ScrollLock, "Scroll Lock"),
        __FORMULA(eKI_Numpad_NumLock, "(Numpad) Num Lock"),
        __FORMULA(eKI_PrintScreen, "Print Screen"),
        F(Pause),

        F(F1), F(F2), F(F3), F(F4), F(F5),
        F(F6), F(F7), F(F8), F(F9), F(F10),
        F(F11), F(F12), F(F13), F(F14), F(F15),
        F(F16), F(F17), F(F18), F(F19), F(F20),
        F(F21), F(F22), F(F23), F(F24), F(F25),

        __FORMULA(eKI_Numpad_0, "(Numpad) 0"),
        __FORMULA(eKI_Numpad_1, "(Numpad) 1"),
        __FORMULA(eKI_Numpad_2, "(Numpad) 2"),
        __FORMULA(eKI_Numpad_3, "(Numpad) 3"),
        __FORMULA(eKI_Numpad_4, "(Numpad) 4"),
        __FORMULA(eKI_Numpad_5, "(Numpad) 5"),
        __FORMULA(eKI_Numpad_6, "(Numpad) 6"),
        __FORMULA(eKI_Numpad_7, "(Numpad) 7"),
        __FORMULA(eKI_Numpad_8, "(Numpad) 8"),
        __FORMULA(eKI_Numpad_9, "(Numpad) 9"),

        __FORMULA(eKI_Numpad_Period, "(Numpad) ."),
        __FORMULA(eKI_Numpad_Divide, "(Numpad) /"),
        __FORMULA(eKI_Numpad_Multiply, "(Numpad) *"),
        __FORMULA(eKI_Numpad_Subtract, "(Numpad) -"),
        __FORMULA(eKI_Numpad_Add, "(Numpad) +"),
        __FORMULA(eKI_Numpad_Enter, "(Numpad) Enter"),
        __FORMULA(eKI_Numpad_Equal, "(Numpad) ="),

        __FORMULA(eKI_LShift, "(L) Shift"),
        __FORMULA(eKI_LCtrl, "(L) Ctrl"),
        __FORMULA(eKI_LAlt, "(L) Alt"),
        __FORMULA(eKI_LSuper, "(L) Super"),
        __FORMULA(eKI_RShift, "(R) Shift"),
        __FORMULA(eKI_RCtrl, "(R) Ctrl"),
        __FORMULA(eKI_RAlt, "(R) Alt"),
        __FORMULA(eKI_RSuper, "(R) Super"),
        F(Menu)
    };

#undef __FORMULA
#undef F

    static const UnicodeString strUnknown = Frame::UTF8Utils::ToUnicode("Unknown");

    auto it = map.find(keyId);
    if(it != map.end()) {
        return it->second;
    }
    return strUnknown;
}

Frame::EKeyId GetAnyKeyPressed() {
    using namespace Frame;
    static constexpr EKeyId keys[] = {
        eKI_Space, eKI_Quote, eKI_Comma, eKI_Subtract, eKI_Period, eKI_Slash,
        eKI_0, eKI_1, eKI_2, eKI_3, eKI_4, eKI_5, eKI_6, eKI_7, eKI_8, eKI_9,
        eKI_Semicolon, eKI_Equal,
        eKI_A, eKI_B, eKI_C, eKI_D, eKI_E, eKI_F, eKI_G, eKI_H, eKI_I, eKI_J, eKI_K, eKI_L, eKI_M,
        eKI_N, eKI_O, eKI_P, eKI_Q, eKI_R, eKI_S, eKI_T, eKI_U, eKI_V, eKI_W, eKI_X, eKI_Y, eKI_Z,
        eKI_LeftBracket, eKI_BackSlash, eKI_RightBracket, eKI_BackQuote,
        eKI_World1_161, eKI_World2_162,
        eKI_Escape, eKI_Enter, eKI_Tab, eKI_Backspace, eKI_Insert, eKI_Delete,
        eKI_Right, eKI_Left, eKI_Down, eKI_Up,
        eKI_PageUp, eKI_PageDown, eKI_Home, eKI_End,
        eKI_CapsLock, eKI_ScrollLock, eKI_Numpad_NumLock,
        eKI_PrintScreen, eKI_Pause,
        eKI_F1, eKI_F2, eKI_F3, eKI_F4, eKI_F5, eKI_F6, eKI_F7, eKI_F8, eKI_F9, eKI_F10, eKI_F11, eKI_F12, eKI_F13,
        eKI_F14, eKI_F15, eKI_F16, eKI_F17, eKI_F18, eKI_F19, eKI_F20, eKI_F21, eKI_F22, eKI_F23, eKI_F24, eKI_F25,
        eKI_Numpad_0, eKI_Numpad_1, eKI_Numpad_2, eKI_Numpad_3, eKI_Numpad_4,
        eKI_Numpad_5, eKI_Numpad_6, eKI_Numpad_7, eKI_Numpad_8, eKI_Numpad_9,
        eKI_Numpad_Period, eKI_Numpad_Divide, eKI_Numpad_Multiply, eKI_Numpad_Subtract, eKI_Numpad_Add, eKI_Numpad_Enter, eKI_Numpad_Equal,
        eKI_LShift, eKI_LCtrl, eKI_LAlt, eKI_LSuper,
        eKI_RShift, eKI_RCtrl, eKI_RAlt, eKI_RSuper,
        eKI_Menu
    };
    static constexpr size_t keysSize = sizeof(keys) / sizeof(* keys);
    
    for(size_t i = 0; i < keysSize; i++) {
        if(Frame::gInput->pKeyboard->GetPressed(keys[i])) {
            return keys[i];
        }
    }

    return eKI_Unknown;
}

void DrawBlockBackground() {
    Frame::Vec2 camPos = Frame::gCamera->GetPos();
    Frame::Vec2 camSizeHalf = Frame::Vec2Cast<float>(Frame::gCamera->GetViewSize()) / Frame::gCamera->GetZoom() * .5f;
    camSizeHalf.y = camSizeHalf.x = std::max(camSizeHalf.x, camSizeHalf.y);
    Frame::Vec2 camLT = camPos - camSizeHalf, camRB = camPos + camSizeHalf;
    float blockSize = 512.f;
    int iBlockSize = int(blockSize);
    Frame::Vec2i over = Frame::Vec2Cast<int>((camSizeHalf * 2.f) / blockSize * 1.414f) + 5;
    Frame::Vec2 offset {
        blockSize * float(int(camLT.x) / iBlockSize),
        blockSize * float(int(camLT.y) / iBlockSize)
    };
    offset -= blockSize * 5.f;

    bool firstFlag = bool(int(camLT.x) / iBlockSize % 2) ^ bool(int(camLT.y) / iBlockSize % 2);
    for(int y = 0; y <= over.y; y++) {
        for(int x = 0; x <= over.x; x++) {
            if((x % 2 == y % 2) ^ firstFlag) continue;
            Frame::Vec2 pos { float(x) * blockSize, float(y) * blockSize };
            pos += offset;
            Frame::gRenderer->pShapeRenderer->DrawRectangleBlended(pos, pos + blockSize, 0x000000, .1f, 0.f);
        }
    }
}

void DrawSpriteBlendedPro(const Frame::SSpriteImage * pSpriteImage, const Frame::Vec2 & vPos, const Frame::ColorRGB & rgb, float alpha, float angle, const Frame::Vec2 & vScale, float angleAfterScale) {
    Frame::STextureVertexBuffer vertexBuffer = Frame::gRenderer->GetTextureVertexBuffer();
    vertexBuffer.SetBlends(rgb, alpha);

    Frame::Vec2 tl = pSpriteImage->GetTopLeftOffset();
    Frame::Vec2 tr = pSpriteImage->GetTopRightOffset();
    Frame::Vec2 bl = pSpriteImage->GetBottomLeftOffset();
    Frame::Vec2 br = pSpriteImage->GetBottomRightOffset();

    Frame::Rotate2DVectorsDegree(angle, { & tl, & tr, & bl, & br });
    tl *= vScale;
    tr *= vScale;
    bl *= vScale;
    br *= vScale;
    Frame::Rotate2DVectorsDegree(angleAfterScale, { & tl, & tr, & bl, & br });

    vertexBuffer.SetPositions(vPos + tl, vPos + tr, vPos + bl, vPos + br);
    vertexBuffer.SetTexCoord(pSpriteImage->GetUVLeftTop(), pSpriteImage->GetUVRightBottom());

    Frame::gRenderer->DrawTexture(pSpriteImage->GetTextureId(), vertexBuffer);
}

void RecursiveMachinePartEditorDevices(std::unordered_set<CEditorDeviceComponent *> * outSet, CEditorDeviceComponent * pComp) {
    if(!outSet || !pComp) {
        return;
    }

    if(outSet->find(pComp) != outSet->end()) {
        return;
    } else {
        outSet->insert(pComp);
    }

    for(const auto & pNeighbor : pComp->m_neighbors) {
        if(!pNeighbor || outSet->find(pNeighbor) != outSet->end()) {
            continue;
        }
        if(IsMachinePartJoint(pNeighbor->GetDeviceType())) {
            continue;
        }
        RecursiveMachinePartEditorDevices(outSet, pNeighbor);
    }
}

void RecursiveMachinePartEditorDevices(std::unordered_set<CEditorDeviceComponent *> * outSet, std::unordered_set<CEditorDeviceComponent *> * outJointSet, CEditorDeviceComponent * pComp, const std::unordered_set<CEditorDeviceComponent *> & ignore) {
    if(!outSet || !outJointSet || !pComp) {
        return;
    }

    if(outSet->find(pComp) != outSet->end() || ignore.find(pComp) != ignore.end()) {
        return;
    } else {
        outSet->insert(pComp);
    }

    if(IsMachinePartJoint(pComp->GetDeviceType())) {
        outJointSet->insert(pComp);

        // 当后面的装置不为关节类装置 或 后方的关节类装置与自己背靠背连接
        if(auto pBack = pComp->m_neighbors[GetRevDirIndex(pComp->GetDirIndex())]; pBack && (!IsMachinePartJoint(pBack->GetDeviceType()) || pComp == pBack->m_neighbors[GetRevDirIndex(pBack->GetDirIndex())])) {
            RecursiveMachinePartEditorDevices(outSet, outJointSet, pBack, ignore);
        }
        return;
    }

    for(const auto & pNeighbor : pComp->m_neighbors) {
        if(!pNeighbor || outSet->find(pNeighbor) != outSet->end() || ignore.find(pNeighbor) != ignore.end()) {
            continue;
        }
        if(IsMachinePartJoint(pNeighbor->GetDeviceType())) {
            if(outJointSet->find(pNeighbor) == outJointSet->end()) {
                outJointSet->insert(pNeighbor);
            }
            continue;
        }
        RecursiveMachinePartEditorDevices(outSet, outJointSet, pNeighbor, ignore);
    }
}

int GetMachinePartJointDevicePointDirIndex(CEditorDeviceComponent * pEDComp) {
    if(!pEDComp) {
        return 0;
    }
    for(int i = 0; i < 4; i++) {
        if(i == GetRevDirIndex(pEDComp->GetDirIndex())) {
            continue;
        }

        if(pEDComp->m_neighbors[i]) {
            return i;
        }
    }
    return pEDComp->GetDirIndex();
}
