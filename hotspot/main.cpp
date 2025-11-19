#include "src/share/vm/prims/jni.h"
#include <stdio.h>

int main(int argc, char** argv) {
    JavaVM* vm;
    JNIEnv* env;

    JavaVMInitArgs vm_args;
    vm_args.version = JNI_VERSION_1_8;
    vm_args.nOptions = 0;
    vm_args.options = nullptr;
    vm_args.ignoreUnrecognized = false;

    jint result = JNI_CreateJavaVM(&vm, (void**)&env, &vm_args);

    if (result == JNI_OK) {
        printf("Java VM created successfully!\n");

        // Destroy the VM
        vm->DestroyJavaVM();
        printf("Java VM destroyed.\n");
    } else {
        printf("Failed to create Java VM. Error code: %d\n", result);
    }

    return 0;
}