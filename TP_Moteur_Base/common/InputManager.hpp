#ifndef INPUTMANAGER_HPP
#define INPUTMANAGER_HPP

#include <GLFW/glfw3.h>
#include <unordered_map>
#include <functional>
#include <string>

struct InputContext {
    std::unordered_map<int, std::function<void()>> holdActions;
    std::unordered_map<int, std::function<void()>> pressActions;
};

class InputManager {
private:
    std::unordered_map<std::string, InputContext> m_contexts;
    std::string m_activeContext;
    std::unordered_map<int, bool> m_previousKeyStates;

public:
    void SetContext(const std::string& contextName) {
        m_activeContext = contextName;
        m_previousKeyStates.clear();
    }

    void BindHold(const std::string& context, int key, std::function<void()> action) {
        m_contexts[context].holdActions[key] = action;
    }

    void BindPress(const std::string& context, int key, std::function<void()> action) {
        m_contexts[context].pressActions[key] = action;
    }

    void Update(GLFWwindow* window) {
        if (m_activeContext.empty() || m_contexts.find(m_activeContext) == m_contexts.end()) return;

        InputContext& currentContext = m_contexts[m_activeContext];

        // 处理持续按压 (Hold)
        for (const auto& pair : currentContext.holdActions) {
            if (glfwGetKey(window, pair.first) == GLFW_PRESS) pair.second();
        }

        // 处理单次按下 (Press)
        for (auto& pair : currentContext.pressActions) {
            int key = pair.first;
            bool isPressed = (glfwGetKey(window, key) == GLFW_PRESS);
            bool wasPressed = m_previousKeyStates[key];

            if (isPressed && !wasPressed) pair.second();
            m_previousKeyStates[key] = isPressed;
        }
    }
};

#endif //INPUTMANAGER_HPP