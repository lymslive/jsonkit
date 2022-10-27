/**
 * @file json_protobuf.cpp
 * @author lymslive
 * @date 2022-10-27
 * @brief Implement conversion between protobuf and json, need protobuf3.
 * */
#ifdef HAS_GOOGLE_PROBUF
#include <google/protobuf/util/json_util.h>
#include <memory>

namespace jsonkit
{

/// Create a protobuf message in general way.
/// The caller take ownership of the returned Message object to delete it.
google::protobuf::Message* CreateMessageByName(const std::string& name)
{
    using namespace google::protobuf;
    Message* pMessage = nullptr;
    const Descriptor* pDescriptor = DescriptorPool::generated_pool()->FindMessageTypeByName(name);
    if (pDescriptor)
    {
        const Message* prototype = MessageFactory::generated_factory()->GetPrototype(pDescriptor);
        if (prototype)
        {
            pMessage = prototype->New();
        }
    }
    return pMessage;
}

bool form_protobuf(const std::string& name, const std::string& inJson, std::string& outPbmsg)
{
    google::protobuf::Message* pMessage = CreateMessageByName(name);
    if (pMessage == nullptr)
    {
        return false;
    }
    std::unique_ptr<google::protobuf::Message> autoDelete(pMessage);

    auto status = google::protobuf::util::JsonStringToMessage(inJson, pMessage);
    if (!status.ok())
    {
        return false;
    }

    if (pMessage->SerializeToString(&outPbmsg) == false)
    {
        return false;
    }

    return true;
}

bool from_protobuf(const std::string& name, const std::string& inPbmsg, std::string& outJson)
{
    google::protobuf::Message* pMessage = CreateMessageByName(name);
    if (pMessage == nullptr)
    {
        return false;
    }
    std::unique_ptr<google::protobuf::Message> autoDelete(pMessage);

    if (pMessage->ParseFromString(inPbmsg) == false)
    {
        return false;
    }

    auto status = google::protobuf::util::MessageToJsonString(*pMessage, &outJson);
    if (!status.ok())
    {
        return false;
    }

    return true;
}

} /* jsonkit */ 
#endif

