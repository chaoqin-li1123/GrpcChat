
#ifndef ASYNC_STREAM_H
#define ASYNC_STREAM_H

enum class Action { INIT, READ, WRITE, FINISH };

template <typename Actor>
struct ActionTag {
  ActionTag(Actor* actor, Action action) : actor_(actor), action_(action) {}
  void act() {
    switch (action_) {
      case Action::INIT:
        actor_->onInitComplete();
        break;
      case Action::READ:
        actor_->onReadComplete();
        break;
      case Action::WRITE:
        actor_->onWriteComplete();
        break;
      case Action::FINISH:
        actor_->onFinishComplete();
        break;
    }
  }
  Actor* actor_;
  Action action_;
};

struct AsyncStream {
  virtual void onReadComplete() = 0;

  virtual void onInitComplete() = 0;

  virtual void onWriteComplete() = 0;

  virtual void onFinishComplete() = 0;
};

#endif