/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2024 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/

#include <modules/webbrowser/webbrowsermodule.h>

#include <inviwo/core/common/inviwoapplication.h>       // for InviwoApplication
#include <inviwo/core/common/inviwomodule.h>            // for ModulePath
#include <inviwo/core/properties/boolproperty.h>        // for BoolProperty
#include <inviwo/core/properties/buttonproperty.h>      // for ButtonProperty
#include <inviwo/core/properties/directoryproperty.h>   // for DirectoryProperty
#include <inviwo/core/properties/fileproperty.h>        // for FileProperty
#include <inviwo/core/properties/minmaxproperty.h>      // for MinMaxProperty
#include <inviwo/core/properties/optionproperty.h>      // for OptionProperty
#include <inviwo/core/properties/ordinalproperty.h>     // for OrdinalProperty
#include <inviwo/core/properties/ordinalrefproperty.h>  // for OrdinalRefProp...
#include <inviwo/core/properties/stringproperty.h>      // for StringProperty
#include <inviwo/core/util/commandlineparser.h>         // for CommandLineParser
#include <inviwo/core/util/exception.h>                 // for ModuleInitExce...
#include <inviwo/core/util/filesystem.h>                // for getExecutablePath
#include <inviwo/core/util/foreacharg.h>                // for for_each_type
#include <inviwo/core/util/glmmat.h>                    // for dmat2, dmat3
#include <inviwo/core/util/glmvec.h>                    // for dvec2, dvec3
#include <inviwo/core/util/logcentral.h>                // for LogCentral
#include <inviwo/core/util/settings/settings.h>         // for Settings
#include <inviwo/core/util/settings/systemsettings.h>   // for SystemSettings
#include <inviwo/core/util/staticstring.h>              // for operator+
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/util/timer.h>                                        // for Timer, Timer::...
#include <inviwo/dataframe/properties/columnoptionproperty.h>              // for ColumnOptionPr...
#include <modules/opengl/shader/shadermanager.h>                           // for ShaderManager
#include <modules/webbrowser/processors/webbrowserprocessor.h>             // for WebBrowserProc...
#include <modules/webbrowser/properties/propertywidgetcef.h>               // for PropertyWidgetCEF
#include <modules/webbrowser/properties/propertywidgetceffactoryobject.h>  // for PropertyWidget...
// Autogenerated
#include <modules/webbrowser/shader_resources.h>    // for addShaderResou...
#include <modules/webbrowser/webbrowserapp.h>       // for WebBrowserApp
#include <modules/webbrowser/webbrowserclient.h>    // for WebBrowserClient
#include <modules/webbrowser/webbrowsersettings.h>  // for WebBrowserSett...

#include <cstddef>      // for size_t, NULL
#include <functional>   // for __base, function
#include <locale>       // for locale
#include <string_view>  // for string_view
#include <tuple>        // for tuple
#include <type_traits>  // for make_unsigned_t
#include <utility>      // for move

#include <glm/fwd.hpp>                 // for dquat, fquat
#include <glm/gtc/type_precision.hpp>  // for i64
#include <glm/mat2x2.hpp>              // for operator+
#include <glm/mat3x3.hpp>              // for operator+
#include <glm/mat4x4.hpp>              // for operator+
#include <glm/vec2.hpp>                // for operator+
#include <glm/vec3.hpp>                // for operator+
#include <glm/vec4.hpp>                // for operator+

#include <fmt/std.h>

#include <warn/push>
#include <warn/ignore/all>
#include "include/cef_app.h"
#include "include/cef_parser.h"
#include <include/cef_base.h>  // for CefSettings
#include <warn/pop>

namespace inviwo {

struct OrdinalCEFWidgetReghelper {
    template <typename T>
    auto operator()(WebBrowserModule& m) {
        using PropertyType = OrdinalProperty<T>;
        m.registerPropertyWidgetCEF<PropertyWidgetCEF, PropertyType>();
    }
};

struct OrdinalRefCEFWidgetReghelper {
    template <typename T>
    auto operator()(WebBrowserModule& m) {
        using PropertyType = OrdinalRefProperty<T>;
        m.registerPropertyWidgetCEF<PropertyWidgetCEF, PropertyType>();
    }
};

struct MinMaxCEFWidgetReghelper {
    template <typename T>
    auto operator()(WebBrowserModule& m) {
        using PropertyType = MinMaxProperty<T>;
        m.registerPropertyWidgetCEF<PropertyWidgetCEF, PropertyType>();
    }
};

struct OptionCEFWidgetReghelper {
    template <typename T>
    auto operator()(WebBrowserModule& m) {
        using PropertyType = OptionProperty<T>;
        m.registerPropertyWidgetCEF<PropertyWidgetCEF, PropertyType>();
    }
};

struct OptionEnumCEFWidgetReghelper {
    template <typename T>
    auto operator()(WebBrowserModule& m) {
        enum class e : T;
        using PropertyType = OptionProperty<e>;
        m.registerPropertyWidgetCEF<PropertyWidgetCEF, PropertyType>();

        enum class eU : std::make_unsigned_t<T>;
        using PropertyTypeU = OptionProperty<eU>;
        m.registerPropertyWidgetCEF<PropertyWidgetCEF, PropertyTypeU>();
    }
};

WebBrowserModule::WebBrowserModule(InviwoApplication* app)
    : InviwoModule(app, "WebBrowser")
    , doChromiumWork_(Timer::Milliseconds(16), []() { CefDoMessageLoopWork(); }) {

    auto moduleSettings = std::make_unique<WebBrowserSettings>();

    moduleSettings->refreshRate_.onChange([this, ptr = moduleSettings.get()]() {
        doChromiumWork_.setInterval(Timer::Milliseconds(1000 / ptr->refreshRate_));
    });
    doChromiumWork_.setInterval(Timer::Milliseconds(1000 / moduleSettings->refreshRate_));

    registerSettings(std::move(moduleSettings));

    // Register widgets
    registerPropertyWidgetCEF<PropertyWidgetCEF, BoolProperty>();
    registerPropertyWidgetCEF<PropertyWidgetCEF, ButtonProperty>();
    registerPropertyWidgetCEF<PropertyWidgetCEF, FileProperty>();
    registerPropertyWidgetCEF<PropertyWidgetCEF, DirectoryProperty>();
    registerPropertyWidgetCEF<PropertyWidgetCEF, StringProperty>();

    registerPropertyWidgetCEF<PropertyWidgetCEF, ColumnOptionProperty>();

    // Register ordinal property widgets
    using OrdinalTypes =
        std::tuple<float, vec2, vec3, vec4, mat2, mat3, mat4, double, dvec2, dvec3, dvec4, dmat2,
                   dmat3, dmat4, int, ivec2, ivec3, ivec4, glm::i64, unsigned int, uvec2, uvec3,
                   uvec4, size_t, size2_t, size3_t, size4_t, glm::fquat, glm::dquat>;

    util::for_each_type<OrdinalTypes>{}(OrdinalCEFWidgetReghelper{}, *this);
    util::for_each_type<OrdinalTypes>{}(OrdinalRefCEFWidgetReghelper{}, *this);

    // Register MinMaxProperty widgets
    using ScalarTypes = std::tuple<float, double, int, glm::i64, size_t>;
    util::for_each_type<ScalarTypes>{}(MinMaxCEFWidgetReghelper{}, *this);

    // Register option property widgets
    using OptionTypes = std::tuple<unsigned int, int, size_t, float, double, std::string>;
    util::for_each_type<OptionTypes>{}(OptionCEFWidgetReghelper{}, *this);

    // Register option property widgets for enums, commented types not yet supported by Inviwo
    using OptionEnumTypes = std::tuple<char, int /*short, long, long long*/>;
    util::for_each_type<OptionEnumTypes>{}(OptionEnumCEFWidgetReghelper{}, *this);

    if (!app->getSystemSettings().enablePickingProperty_) {
        LogInfo(
            "Enabling picking system setting since it is required for interaction "
            "(View->Settings->System settings->Enable picking).");
        app->getSystemSettings().enablePickingProperty_.set(true);
    }
    // CEF initialization
    // Specify the path for the sub-process executable.
    auto exeExtension = filesystem::getExecutablePath().extension();
    // Assume that inviwo_web_helper is next to the main executable
    auto exeDirectory = filesystem::getExecutablePath().parent_path();

    auto locale = app->getUILocale().name();
    if (locale == "C") {
        // Crash when default locale "C" is used. Reproduce with GLFWMinimum application
        locale = std::locale("en_US.UTF-8").name();
    }

    void* sandbox_info = NULL;  // Windows specific

#ifdef __APPLE__  // Mac specific

    // Find CEF framework and helper app in
    // exe.app/Contents/Frameworks directory first
    auto cefParentDir = std::filesystem::weakly_canonical(exeDirectory / "..");
    auto frameworkDirectory = cefParentDir / "Frameworks/Chromium Embedded Framework.framework";
    auto frameworkPath = frameworkDirectory / "Chromium Embedded Framework";
    // Load the CEF framework library at runtime instead of linking directly
    // as required by the macOS sandbox implementation.
    if (!cefLib_.LoadInMain()) {
        throw ModuleInitException("Could not find Chromium Embedded Framework.framework: " +
                                  frameworkPath.string());
    }

    CefSettings settings;
    // Setting locales_dir_path does not seem to work (tested debug mode with Xcode).
    // We have therefore created symbolic links from the bundle Resources directory to the
    // framework resource files using CMake.
    // See documentation about settings in cef_types.h:
    // "This value [locales_dir_path] is ignored on MacOS where pack files are always
    // loaded from the app bundle Resources directory".
    //
    // We still set the variable to potentially avoid other problems such as the one below.
    // Crashes if not set and non-default locale is used
    CefString(&settings.locales_dir_path).FromWString((frameworkDirectory / "Resources").wstring());

    // resources_dir_path specified location of:
    // cef.pak
    // cef_100_percent.pak
    // cef_200_percent.pak
    //      These files contain non-localized resources used by CEF, Chromium and Blink.
    //      Without these files arbitrary Web components may display incorrectly.
    //
    // cef_extensions.pak
    //      This file contains non-localized resources required for extension loading.
    //      Pass the `--disable-extensions` command-line flag to disable use of this
    //      file. Without this file components that depend on the extension system,
    //      such as the PDF viewer, will not function.
    //
    // devtools_resources.pak
    //      This file contains non-localized resources required for Chrome Developer
    //      Tools. Without this file Chrome Developer Tools will not function.
    CefString(&settings.resources_dir_path)
        .FromWString((frameworkDirectory / "Resources").wstring());
    // Locale returns "en_US.UFT8" but "en.UTF8" is needed by CEF
    auto startErasePos = locale.find('_');
    if (startErasePos != std::string::npos) {
        locale.erase(startErasePos, locale.find('.') - startErasePos);
    }

#else
    CefSettings settings;
    // Non-mac systems uses a single helper executable so here we can specify name
    // Linux will have empty extension
    auto subProcessExecutable = exeDirectory / "cef_web_helper";
    subProcessExecutable += exeExtension;
    if (!std::filesystem::is_regular_file(subProcessExecutable)) {
        throw ModuleInitException(
            fmt::format("Could not find web helper executable: {}", subProcessExecutable));
    }

    // Necessary to run helpers in separate sub-processes
    // Needed since we do not want to edit the "main" function
    CefString(&settings.browser_subprocess_path).FromWString(subProcessExecutable.wstring());
#endif

#ifdef WIN32
#if defined(CEF_USE_SANDBOX)
    // Manage the life span of the sandbox information object. This is necessary
    // for sandbox support on Windows. See cef_sandbox_win.h for complete details.
    CefScopedSandboxInfo scoped_sandbox;
    sandbox_info = scoped_sandbox.sandbox_info();
#endif
#endif
    // When generating projects with CMake the CEF_USE_SANDBOX value will be defined
    // automatically. Pass -DUSE_SANDBOX=OFF to the CMake command-line to disable
    // use of the sandbox.
#if !defined(CEF_USE_SANDBOX)
    settings.no_sandbox = true;
#endif
    // checkout detailed settings options
    // http://magpcss.org/ceforum/apidocs/projects/%28default%29/_cef_settings_t.html nearly all
    // the settings can be set via args too.
    settings.multi_threaded_message_loop = false;  // not supported, except windows

    // We want to use off-screen rendering
    settings.windowless_rendering_enabled = true;

    // Let the Inviwo application (Qt/GLFW) handle operating system event processing
    // instead of CEF. Setting external message pump to false will cause mouse events
    // to be processed in CefDoMessageLoopWork() instead of in the Qt application loop.
    settings.external_message_pump = true;

    CefString(&settings.locale).FromString(locale);

    // Optional implementation of the CefApp interface.
    CefRefPtr<WebBrowserApp> browserApp(new WebBrowserApp);

    CefMainArgs args;
    if (!CefInitialize(args, settings, browserApp, sandbox_info)) {
        throw ModuleInitException("Failed to initialize Chromium Embedded Framework");
    }

    // Add a directory to the search path of the ShaderManager
    webbrowser::addShaderResources(ShaderManager::getPtr(), {getPath(ModulePath::GLSL)});
    // ShaderManager::getPtr()->addShaderSearchPath(getPath(ModulePath::GLSL));

    // Register objects that can be shared with the rest of inviwo here:

    // Processors
    registerProcessor<WebBrowserProcessor>();

    doChromiumWork_.start();

    browserClient_ = new WebBrowserClient(app->getModuleManager(), getPropertyWidgetCEFFactory());
}

WebBrowserModule::~WebBrowserModule() {
    // Stop message pumping and make sure that app has finished processing before CefShutdown
    doChromiumWork_.stop();
    app_->waitForPool();
    CefShutdown();
}

std::string WebBrowserModule::getDataURI(const std::string& data, const std::string& mime_type) {
    return "data:" + mime_type + ";base64," +
           CefURIEncode(CefBase64Encode(data.data(), data.size()), false).ToString();
}

std::string WebBrowserModule::getCefErrorString(cef_errorcode_t code) {
    switch (code) {
        case ERR_NONE:
            return "ERR_NONE";
        case ERR_FAILED:
            return "ERR_FAILED";
        case ERR_ABORTED:
            return "ERR_ABORTED";
        case ERR_INVALID_ARGUMENT:
            return "ERR_INVALID_ARGUMENT";
        case ERR_INVALID_HANDLE:
            return "ERR_INVALID_HANDLE";
        case ERR_FILE_NOT_FOUND:
            return "ERR_FILE_NOT_FOUND";
        case ERR_TIMED_OUT:
            return "ERR_TIMED_OUT";
        case ERR_FILE_TOO_BIG:
            return "ERR_FILE_TOO_BIG";
        case ERR_UNEXPECTED:
            return "ERR_UNEXPECTED";
        case ERR_ACCESS_DENIED:
            return "ERR_ACCESS_DENIED";
        case ERR_NOT_IMPLEMENTED:
            return "ERR_NOT_IMPLEMENTED";
        case ERR_CONNECTION_CLOSED:
            return "ERR_CONNECTION_CLOSED";
        case ERR_CONNECTION_RESET:
            return "ERR_CONNECTION_RESET";
        case ERR_CONNECTION_REFUSED:
            return "ERR_CONNECTION_REFUSED";
        case ERR_CONNECTION_ABORTED:
            return "ERR_CONNECTION_ABORTED";
        case ERR_CONNECTION_FAILED:
            return "ERR_CONNECTION_FAILED";
        case ERR_NAME_NOT_RESOLVED:
            return "ERR_NAME_NOT_RESOLVED";
        case ERR_INTERNET_DISCONNECTED:
            return "ERR_INTERNET_DISCONNECTED";
        case ERR_SSL_PROTOCOL_ERROR:
            return "ERR_SSL_PROTOCOL_ERROR";
        case ERR_ADDRESS_INVALID:
            return "ERR_ADDRESS_INVALID";
        case ERR_ADDRESS_UNREACHABLE:
            return "ERR_ADDRESS_UNREACHABLE";
        case ERR_SSL_CLIENT_AUTH_CERT_NEEDED:
            return "ERR_SSL_CLIENT_AUTH_CERT_NEEDED";
        case ERR_TUNNEL_CONNECTION_FAILED:
            return "ERR_TUNNEL_CONNECTION_FAILED";
        case ERR_NO_SSL_VERSIONS_ENABLED:
            return "ERR_NO_SSL_VERSIONS_ENABLED";
        case ERR_SSL_VERSION_OR_CIPHER_MISMATCH:
            return "ERR_SSL_VERSION_OR_CIPHER_MISMATCH";
        case ERR_SSL_RENEGOTIATION_REQUESTED:
            return "ERR_SSL_RENEGOTIATION_REQUESTED";
        case ERR_CERT_COMMON_NAME_INVALID:
            return "ERR_CERT_COMMON_NAME_INVALID";
        case ERR_CERT_DATE_INVALID:
            return "ERR_CERT_DATE_INVALID";
        case ERR_CERT_AUTHORITY_INVALID:
            return "ERR_CERT_AUTHORITY_INVALID";
        case ERR_CERT_CONTAINS_ERRORS:
            return "ERR_CERT_CONTAINS_ERRORS";
        case ERR_CERT_NO_REVOCATION_MECHANISM:
            return "ERR_CERT_NO_REVOCATION_MECHANISM";
        case ERR_CERT_UNABLE_TO_CHECK_REVOCATION:
            return "ERR_CERT_UNABLE_TO_CHECK_REVOCATION";
        case ERR_CERT_REVOKED:
            return "ERR_CERT_REVOKED";
        case ERR_CERT_INVALID:
            return "ERR_CERT_INVALID";
        case ERR_CERT_END:
            return "ERR_CERT_END";
        case ERR_INVALID_URL:
            return "ERR_INVALID_URL";
        case ERR_DISALLOWED_URL_SCHEME:
            return "ERR_DISALLOWED_URL_SCHEME";
        case ERR_UNKNOWN_URL_SCHEME:
            return "ERR_UNKNOWN_URL_SCHEME";
        case ERR_TOO_MANY_REDIRECTS:
            return "ERR_TOO_MANY_REDIRECTS";
        case ERR_UNSAFE_REDIRECT:
            return "ERR_UNSAFE_REDIRECT";
        case ERR_UNSAFE_PORT:
            return "ERR_UNSAFE_PORT";
        case ERR_INVALID_RESPONSE:
            return "ERR_INVALID_RESPONSE";
        case ERR_INVALID_CHUNKED_ENCODING:
            return "ERR_INVALID_CHUNKED_ENCODING";
        case ERR_METHOD_NOT_SUPPORTED:
            return "ERR_METHOD_NOT_SUPPORTED";
        case ERR_UNEXPECTED_PROXY_AUTH:
            return "ERR_UNEXPECTED_PROXY_AUTH";
        case ERR_EMPTY_RESPONSE:
            return "ERR_EMPTY_RESPONSE";
        case ERR_RESPONSE_HEADERS_TOO_BIG:
            return "ERR_RESPONSE_HEADERS_TOO_BIG";
        case ERR_CACHE_MISS:
            return "ERR_CACHE_MISS";
        case ERR_INSECURE_RESPONSE:
            return "ERR_INSECURE_RESPONSE";
        default:
            return "UNKNOWN";
    }
}
}  // namespace inviwo
