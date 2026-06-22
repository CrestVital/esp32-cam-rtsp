"""
Custom PlatformIO test runner for the [env:native] environment.

Purpose: disable PlatformIO's automatic injection of throwtheswitch/Unity
(EXTRA_LIB_DEPS = None) to prevent linker conflicts with the project-local
Unity at third-party/unity/. Each suite's source files are assembled in
configure_build_env() so that PlatformIO compiles the correct component
sources and mock files for that suite.
"""

import posixpath

from platformio.public import UnityTestRunner


# Mapping: suite directory name -> extra source files to compile
# (paths relative to the project root).
# The test file itself is compiled automatically by PlatformIO because
# it lives in the suite directory. unity.c and the mock .c files must
# be listed here because they reside outside the suite directory.
_SUITE_SOURCES = {
    "test_nvs_config": [
        "third-party/unity/unity.c",
        "test/mocks/mock_esp_log_counters.c",
        "test/mocks/mock_nvs_state.c",
        "components/nvs_config/nvs_config.c",
    ],
    "test_app_event": [
        "third-party/unity/unity.c",
        "test/mocks/mock_esp_event.c",
        "test/mocks/mock_esp_log_counters.c",
        "components/app_event/app_event.c",
    ],
    "test_power_manager": [
        "third-party/unity/unity.c",
        "test/mocks/mock_esp_event.c",
        "test/mocks/mock_esp_log_counters.c",
        "test/mocks/mock_power_helpers.c",
        "components/app_event/app_event.c",
        "components/power_manager/power_manager.c",
    ],
}

# Per-suite include paths (added on top of the common flags in platformio.ini).
_SUITE_INCLUDES = {
    "test_nvs_config": [
        "components/nvs_config/include",
    ],
    "test_app_event": [
        "components/app_event/include",
    ],
    "test_power_manager": [
        "components/app_event/include",
        "components/power_manager/include",
        "components/sys_log/include",
    ],
}


class CustomTestRunner(UnityTestRunner):
    # Prevent PlatformIO from downloading / compiling throwtheswitch/Unity.
    # The project ships its own Unity at third-party/unity/.
    EXTRA_LIB_DEPS = None

    def configure_build_env(self, env):
        # Determine which suite is being built. PlatformIO 6 exposes the
        # suite path in PIOTEST_RUNNING_NAME (e.g. "native/test_nvs_config").
        suite_name = env.get("PIOTEST_RUNNING_NAME", "")
        # Extract the leaf directory name.
        suite_key = suite_name.split("/")[-1] if "/" in suite_name else suite_name

        extra_srcs = _SUITE_SOURCES.get(suite_key, [])
        extra_incs = _SUITE_INCLUDES.get(suite_key, [])

        if not extra_srcs:
            # Log clearly so failures are visible in the report.
            print(
                "[CustomTestRunner] WARNING: no sources mapped for suite "
                "key '{}' (PIOTEST_RUNNING_NAME='{}'). "
                "Build may fail with undefined references.".format(
                    suite_key, suite_name
                )
            )

        # Add per-suite include directories to the compiler flags.
        for inc in extra_incs:
            env.Append(CCFLAGS=["-I" + inc])

        # Add the extra .c files to the build graph via BuildSources
        # so they are compiled and linked into the test binary.
        # Files are grouped by their parent directory because BuildSources
        # takes a single source directory per call.
        if extra_srcs:
            by_dir = {}
            for src in extra_srcs:
                d = posixpath.dirname(src)
                f = posixpath.basename(src)
                by_dir.setdefault(d, []).append(f)
            proj_dir = env.subst("$PROJECT_DIR")
            for src_dir, files in by_dir.items():
                env.BuildSources(
                    "$BUILD_DIR/" + src_dir,
                    posixpath.join(proj_dir, src_dir),
                    ["+<{}>".format(f) for f in files]
                )