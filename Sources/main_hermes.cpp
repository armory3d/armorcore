
#include <memory>
#include <string>
#include <jsi/jsi.h>
#include <hermes/VM/static_h.h>
#include <hermes.h>

class MyArrayBuffer : public facebook::jsi::MutableBuffer {
public:
    uint8_t *buffer;
	uint32_t length;
    size_t size() const override;
    uint8_t *data() override;
};

size_t MyArrayBuffer::size() const {
	return length;
}

uint8_t *MyArrayBuffer::data() {
	return buffer;
}

extern "C" void js_eval(SHRuntime *shr, char *str) {
	std::string script = str;
	auto buffer = std::make_shared<const facebook::jsi::StringBuffer>(script);
	facebook::hermes::HermesRuntime *runtime = _sh_get_hermes_runtime(shr);
	runtime->evaluateJavaScript(buffer, "");
	// auto pjs = runtime->prepareJavaScript(buffer, "");
	// runtime->evaluatePreparedJavaScript(pjs);
}

extern "C" void js_call(SHRuntime *shr, char *fn_name) {
	facebook::hermes::HermesRuntime *runtime = _sh_get_hermes_runtime(shr);
	facebook::jsi::Function fn = runtime->global().getPropertyAsFunction(*runtime, fn_name);
	// facebook::jsi::Value param = facebook::jsi::Value(*runtime, facebook::jsi::String::createFromUtf8(*runtime, "text"));
	facebook::jsi::Value param = facebook::jsi::Value(0);
    facebook::jsi::Value result = fn.call(*runtime, std::move(param), 1);
}

extern "C" void js_array_buffer(SHRuntime *shr, uint8_t *buffer, uint32_t length) {
	auto b = std::make_shared<MyArrayBuffer>();
	b->buffer = buffer;
	b->length = length;
	facebook::hermes::HermesRuntime *runtime = _sh_get_hermes_runtime(shr);
	auto ab = facebook::jsi::ArrayBuffer(*runtime, b);
	runtime->global().setProperty(*runtime, "_arraybuffer", ab);
}
