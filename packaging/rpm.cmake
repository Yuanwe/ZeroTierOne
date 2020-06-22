set(CPACK_PACKAGE_VENDOR "ZeroTier, Inc.")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "zerotier")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_PACKAGE_VERSION_RELEASE ${PROJECT_VERSION_RELEASE})
set(CPACK_RPM_PACKAGE_DESCRIPTION "ZeroTier network virtualization service ZeroTier One lets you join ZeroTier virtual networks and have them appear as tun/tap ports on your system. See https://www.zerotier.com/ for instructions and documentation.")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_GENERATOR "RPM")
set(CPACK_RPM_FILE_NAME "RPM-DEFAULT")
set(CPACK_RPM_PACKAGE_RELEASE_DIST "%{dist}")
set(CPACK_RPM_PACKAGE_VENDOR "ZeroTier, Inc.")
set(CPACK_RPM_PACKAGE_AUTOREQ 1)
set(CPACK_RPM_PACKAGE_REQUIRES "systemd, iproute, libstdc++")
set(CPACK_RPM_PACKAGE_REQUIRES_PRE "/usr/sbin/useradd, /usr/bin/getent")
set(CPACK_RPM_PACKAGE_OBSOLETES "zerotier-one")
set(CPACK_PACKAGE_RELOCATABLE FALSE)

set(CPACK_RPM_PRE_INSTALL_SCRIPT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/packaging/rpm/rpm.pre")
set(CPACK_RPM_POST_INSTALL_SCRIPT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/packaging/rpm/rpm.post")
set(CPACK_RPM_POST_UNINSTALL_SCRIPT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/packaging/rpm/rpm.postun")
set(CPACK_RPM_PRE_UNINSTALL_SCRIPT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/packaging/rpm/rpm.preun")
set(CPACK_RPM_CHANGELOG_FILE" ${CMAKE_CURRENT_SOURCE_DIR}/packaging/rpm/rpm.changelog")

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/zerotier DESTINATION bin)
if(IS_DIRECTORY /lib/systemd/system)
    install(
        FILES ${CMAKE_CURRENT_SOURCE_DIR}/packaging/debian/zerotier.service 
        DESTINATION /lib/systemd/system
    )
elseif(IS_DIRECTORY /usr/lib/systemd/system)
    install(
        FILES ${CMAKE_CURRENT_SOURCE_DIR}/packaging/debian/zerotier.service 
        DESTINATION /usr/lib/systemd/system
    )
else()
    install(
        FILES ${CMAKE_CURRENT_SOURCE_DIR}/packaging/debian/zerotier.init
        DESTINATION /etc/init.d
    )
endif()

include(CPack)
