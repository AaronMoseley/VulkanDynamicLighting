#include "ThirdPartyDeclarations.h"
#include <memory>

#include <QApplication>
#include <QVulkanInstance>

#include "VoltEngine.h"

bool DebugFilter(QVulkanInstance::DebugMessageSeverityFlags severity, QVulkanInstance::DebugMessageTypeFlags type, const void* message)
{
    auto* data = reinterpret_cast<const VkDebugUtilsMessengerCallbackDataEXT*>(message);

    qDebug().noquote()
        << "[VK]"
        << "Severity:" << severity
        << "Type:" << type
        << data->pMessage;

    return true;   // return true to stop Qt from printing it as well
}

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    QVulkanInstance instance;
    instance.setLayers({ "VK_LAYER_KHRONOS_validation" });
    instance.setApiVersion(QVersionNumber(1, 3, 0));
    instance.setExtensions({
        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
#ifdef _DEBUG
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME
#endif
    });
    instance.installDebugOutputFilter(DebugFilter);

    bool success = instance.create();

    if (!success)
    {
        qDebug() << "Failed to create Vulkan instance";
        return -1;
    }

    auto screenRect = QRect(0, 0, 915, 400);
    qDebug() << "Screen Size: " << screenRect.width() << " x " << screenRect.height();

    VoltEngine renderingApp(nullptr, &instance, screenRect.width(), screenRect.height());
    renderingApp.show();

    return app.exec();
}