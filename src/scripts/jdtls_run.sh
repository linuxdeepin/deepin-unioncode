#!/bin/bash
/usr/bin/java \
-Declipse.application=org.eclipse.jdt.ls.core.id1 \
-Dosgi.bundles.defaultStartLevel=4 \
-Declipse.product=org.eclipse.jdt.ls.core.product \
-Dlog.level=ALL \
-noverify \
-Xmx1G \
--add-modules=ALL-SYSTEM \
--add-opens \
java.base/java.util=ALL-UNNAMED \
--add-opens java.base/java.lang=ALL-UNNAMED \
-jar $1/plugins/org.eclipse.equinox.launcher_1.6.400.v20210924-0641.jar \
-configuration $1/config_linux \
-data ~/.data
