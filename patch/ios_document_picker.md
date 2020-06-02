
--- a/Backends/System/iOS/Sources/Kore/GLViewController.h
+++ b/Backends/System/iOS/Sources/Kore/GLViewController.h

-@interface GLViewController : UIViewController {
+@interface GLViewController : UIViewController <UIDocumentPickerDelegate> {


--- a/Backends/System/iOS/Sources/Kore/GLViewController.mm
+++ b/Backends/System/iOS/Sources/Kore/GLViewController.mm

+#include <kinc/system.h>

+- (void)documentPicker:(UIDocumentPickerViewController *)controller didPickDocumentAtURL:(NSURL *)url {
+       wchar_t* filePath = (wchar_t*)[url.path cStringUsingEncoding:NSUTF32LittleEndianStringEncoding];
+       CFURLRef cfurl = (__bridge CFURLRef)url;
+       CFURLStartAccessingSecurityScopedResource(cfurl);
+       kinc_internal_drop_files_callback(filePath);
+       CFURLStopAccessingSecurityScopedResource(cfurl);
+}
+


--- a/Backends/System/iOS/Sources/Kore/ios.plist
+++ b/Backends/System/iOS/Sources/Kore/ios.plist

+       <key>UIFileSharingEnabled</key>
+       <true/>
+       <key>LSSupportsOpeningDocumentsInPlace</key>
+       <true/>
