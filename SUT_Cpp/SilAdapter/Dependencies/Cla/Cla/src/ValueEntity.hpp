// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <atomic>
#include <map>
#include <memory>
#include <mutex>
#include <vector>

#include "spdlog/logger.h"

#include "cla/IValueEntity.hpp"

#include "Deserializer.hpp"
#include "Serializer.hpp"

namespace Vector {
namespace CANoe {
namespace Cla {

/*! \brief Class representing the a value entity, which stores serialized data
 *         in a buffer. */
class ValueEntity : public IValueEntity
{
public:
  // -------------------
  // ValueEntity
  // -------------------

  /*! \brief Instantiates a new value entity.
   *  \param logger The logger used by this value entity for logging. */
  ValueEntity(std::shared_ptr<spdlog::logger> logger);

  /*! \brief Sets the content of the value entities buffer.
   *         The storage is locked while the data is set.
   *  \param newData The new buffer data. */
  void SetData(std::vector<uint8_t> newData);

  /*! \brief Returns a copy of data stored in the value entity.
   *         The storage is locked while data is copied.
   *  \returns the data of the value entity. */
  std::vector<uint8_t> CopyData() const;

  // -------------------
  // IValueEntity
  // -------------------

  ISerializer* GetSerializer() const override;
  IDeserializer* GetDeserializer() const override;

  bool IsValid() const override;
  ValueState GetState() const override;
  Time GetLastUpdateTime() const override;
  Time GetLastChangeTime() const override;

  CallbackHandle RegisterCallback(std::function<void(IValueEntity&)> callback, ValueUpdateMode mode) override;
  void UnregisterCallback(CallbackHandle handle) override;

private:
  // Nested Classes
  // ------------------

  struct Callback
  {
    Callback(CallbackHandle handle, std::function<void(IValueEntity&)> function) :
      handle(handle),
      function(std::move(function))
    {
    }

    CallbackHandle handle;
    std::function<void(IValueEntity&)> function;
  };

  // Private Members
  // -------------------

  std::shared_ptr<spdlog::logger> mLogger;
  std::unique_ptr<Serializer> mSerializer;
  std::unique_ptr<Deserializer> mDeserializer;

  std::vector<uint8_t> mData;
  Time mLastUpdateTime = 0;
  Time mLastChangeTime = 0;
  mutable std::mutex mDataMutex;

  std::mutex mCallbackMutex;
  std::atomic<CallbackHandle> mNextCallbackHandle;
  std::vector<std::shared_ptr<Callback>> mOnChangeCallbacks;
  std::vector<std::shared_ptr<Callback>> mOnUpdateCallbacks;

  // Private Methods
  // -------------------

  CallbackHandle CreateUniqueCallbackHandle();

  void OnChange();
  void OnUpdate();
};

} // namespace Cla
} // namespace CANoe
} // namespace Vector
