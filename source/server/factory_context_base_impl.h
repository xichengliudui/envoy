#pragma once

#include "envoy/server/factory_context.h"

namespace Envoy {
namespace Server {

class FactoryContextBaseImpl : public Configuration::FactoryContextBase {
public:
  FactoryContextBaseImpl(const Server::Options& options, Event::Dispatcher& main_thread_dispatcher,
                         Api::Api& api, const LocalInfo::LocalInfo& local_info,
                         Server::Admin& admin, Runtime::Loader& runtime,
                         Singleton::Manager& singleton_manager,
                         ProtobufMessage::ValidationVisitor& validation_visitor,
                         Stats::Store& scope, ThreadLocal::Instance& local)
      : options_(options), main_thread_dispatcher_(main_thread_dispatcher), api_(api),
        local_info_(local_info), admin_(admin), runtime_(runtime),
        singleton_manager_(singleton_manager), validation_visitor_(validation_visitor),
        scope_(scope), thread_local_(local) {}

  // FactoryContextBase
  const Options& options() override { return options_; };
  Event::Dispatcher& mainThreadDispatcher() override { return main_thread_dispatcher_; };
  Api::Api& api() override { return api_; };
  const LocalInfo::LocalInfo& localInfo() const override { return local_info_; };
  Server::Admin& admin() override { return admin_; };
  Envoy::Runtime::Loader& runtime() override { return runtime_; };
  Singleton::Manager& singletonManager() override { return singleton_manager_; };
  ProtobufMessage::ValidationVisitor& messageValidationVisitor() override {
    return validation_visitor_;
  };
  Stats::Scope& scope() override { return scope_; };
  ThreadLocal::SlotAllocator& threadLocal() override { return thread_local_; };

private:
  const Server::Options& options_;
  Event::Dispatcher& main_thread_dispatcher_;
  Api::Api& api_;
  const LocalInfo::LocalInfo& local_info_;
  Server::Admin& admin_;
  Runtime::Loader& runtime_;
  Singleton::Manager& singleton_manager_;
  ProtobufMessage::ValidationVisitor& validation_visitor_;
  Stats::Store& scope_;
  ThreadLocal::Instance& thread_local_;
};

} // namespace Server
} // namespace Envoy
