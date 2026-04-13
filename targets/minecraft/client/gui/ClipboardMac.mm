#include <string>
#import <Cocoa/Cocoa.h>

extern "C" {

std::wstring mac_getClipboard() {
    NSPasteboard* pasteboard = [NSPasteboard generalPasteboard];
    NSArray* classes = @[[NSString class]];
    NSDictionary* options = @{};
    NSArray* contents = [pasteboard readObjectsForClasses:classes options:options];
    
    if (contents && [contents count] > 0) {
        NSString* nsString = (NSString*)[contents objectAtIndex:0];
        if (nsString) {
            const char* utf8String = [nsString UTF8String];
            size_t length = strlen(utf8String);
            std::wstring result;
            result.reserve(length);
            
            for (size_t i = 0; i < length; ++i) {
                result += (wchar_t)(unsigned char)utf8String[i];
            }
            return result;
        }
    }
    return std::wstring();
}

void mac_setClipboard(const std::wstring& str) {
    NSPasteboard* pasteboard = [NSPasteboard generalPasteboard];
    [pasteboard clearContents];
    
    NSMutableString* nsString = [NSMutableString string];
    for (wchar_t ch : str) {
        [nsString appendFormat:@"%c", (char)ch];
    }
    
    [pasteboard setString:nsString forType:NSPasteboardTypeString];
}

}  // extern "C"
