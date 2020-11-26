import org.jetbrains.compose.compose

val composeVersion = System.getenv("COMPOSE_TEMPLATE_COMPOSE_VERSION") ?: "0.2.0-build128"

plugins {
    id("com.android.library")
    kotlin("multiplatform")
    id("org.jetbrains.compose")
}

kotlin {
    android()
    jvm("desktop")

    sourceSets {
        named("commonMain") {
            dependencies {
                api(compose.runtime)
                api(compose.foundation)
                api(compose.material)
                api(compose.animation)
                api(compose.ui)
                api(compose.materialIconsExtended)
            }
        }
        named("androidMain") {
            dependencies {
                api("androidx.appcompat:appcompat:1.2.0")
                api("androidx.core:core-ktx:1.3.2")
                implementation("androidx.lifecycle:lifecycle-viewmodel-ktx:2.2.0")
                implementation("androidx.ui:ui-tooling:$composeVersion")
                implementation("org.androidaudioplugin:androidaudioplugin:0.6.1")
            }
        }
    }
}

android {
    compileSdkVersion(30)

    defaultConfig {
        minSdkVersion(29)
        targetSdkVersion(30)
        versionCode = 1
        versionName = "1.0"
    }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_1_8
        targetCompatibility = JavaVersion.VERSION_1_8
    }

    sourceSets {
        named("main") {
            manifest.srcFile("src/androidMain/AndroidManifest.xml")
            res.srcDirs("src/androidMain/res")
        }
    }
}

