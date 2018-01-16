#include "qt/window/project_wizzard/QtProjectWizzardContentPreferences.h"

#include <QCheckBox>
#include <QComboBox>
#include <QFontComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QTextCodec>

#include "qt/utility/utilityQt.h"
#include "settings/ApplicationSettings.h"
#include "utility/file/FileSystem.h"
#include "utility/messaging/type/MessageSwitchColorScheme.h"
#include "utility/ResourcePaths.h"
#include "utility/utilityPathDetection.h"
#include "utility/utilityApp.h"
#include "utility/utility.h"

QtProjectWizzardContentPreferences::QtProjectWizzardContentPreferences(
	QtProjectWizzardWindow* window
)
	: QtProjectWizzardContent(window)
	, m_oldColorSchemeIndex(-1)
	, m_newColorSchemeIndex(-1)
	, m_screenAutoScaling(nullptr)
	, m_screenScaleFactor(nullptr)
{
	m_colorSchemePaths =
		FileSystem::getFilePathsFromDirectory(ResourcePaths::getColorSchemesPath(), std::vector<std::string>(1, ".xml"));
}

QtProjectWizzardContentPreferences::~QtProjectWizzardContentPreferences()
{
	if (m_oldColorSchemeIndex != -1 && m_oldColorSchemeIndex != m_newColorSchemeIndex)
	{
		colorSchemeChanged(m_oldColorSchemeIndex);
	}
}

void QtProjectWizzardContentPreferences::populate(QGridLayout* layout, int& row)
{
	ApplicationSettings* appSettings = ApplicationSettings::getInstance().get();

	// ui
	addTitle("USER INTERFACE", layout, row);

	// font face
	m_fontFace = new QFontComboBox(this);
	m_fontFace->setFontFilters(QFontComboBox::MonospacedFonts);
	m_fontFace->setEditable(false);
	addLabelAndWidget("Font Face", m_fontFace, layout, row);
	row++;

	// font size
	m_fontSize = addComboBox("Font Size", appSettings->getFontSizeMin(), appSettings->getFontSizeMax(), "", layout, row);

	// tab width
	m_tabWidth = addComboBox("Tab Width", 1, 16, "", layout, row);

	// text encoding
	m_textEncoding = addComboBox("Text Encoding", "", layout, row);
	for (int mib : QTextCodec::availableMibs())
	{
		m_textEncoding->addItem(QTextCodec::codecForMib(mib)->name());
	}

	// color scheme
	m_colorSchemes = addComboBox("Color Scheme", "", layout, row);
	for (size_t i = 0; i < m_colorSchemePaths.size(); i++)
	{
		m_colorSchemes->insertItem(i, m_colorSchemePaths[i].withoutExtension().fileName().c_str());
	}
	connect(m_colorSchemes, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated),
		this, &QtProjectWizzardContentPreferences::colorSchemeChanged);

	// animations
	m_useAnimations = addCheckBox("Animations", "Enable animations",
		"<p>Enable animations throughout the user interface.</p>", layout, row);

	// built-in types
	m_showBuiltinTypes = addCheckBox("Built-in Types", "Show built-in types in graph when referenced",
		"<p>Enable display of referenced built-in types in the graph view.</p>", layout, row);

	addGap(layout, row);


	// Linux UI scale
	// if (utility::getOsType() == OS_LINUX)
	{
		// screen
		addTitle("SCREEN", layout, row);

		QLabel* hint = new QLabel("<changes need restart>");
		hint->setStyleSheet("color: grey");
		layout->addWidget(hint, row-1, QtProjectWizzardWindow::BACK_COL, Qt::AlignRight);

		// auto scaling
		m_screenAutoScalingInfoLabel = new QLabel("");
		m_screenAutoScaling = addComboBoxWithWidgets(
			"Auto Scaling to DPI",
			"<p>Define if automatic scaling to screen DPI resolution is active. "
			"This setting manipulates the environment flag QT_AUTO_SCREEN_SCALE_FACTOR of the Qt framework "
			"(<a href=\"http://doc.qt.io/qt-5/highdpi.html\">http://doc.qt.io/qt-5/highdpi.html</a>). "
			"Choose 'system' to stick to the setting of your current environment.</p>"
			"<p>Changes to this setting require a restart of the application to take effect.</p>",
			{ m_screenAutoScalingInfoLabel },
			layout,
			row
		);
		m_screenAutoScaling->addItem("system", -1);
		m_screenAutoScaling->addItem("off", 0);
		m_screenAutoScaling->addItem("on", 1);
		connect(m_screenAutoScaling, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated),
			this, &QtProjectWizzardContentPreferences::uiAutoScalingChanges);

		// scale factor
		m_screenScaleFactorInfoLabel = new QLabel("");
		m_screenScaleFactor = addComboBoxWithWidgets(
			"Scale Factor",
			"<p>Define a screen scale factor for the user interface of the application. "
			"This setting manipulates the environment flag QT_SCALE_FACTOR of the Qt framework "
			"(<a href=\"http://doc.qt.io/qt-5/highdpi.html\">http://doc.qt.io/qt-5/highdpi.html</a>). "
			"Choose 'system' to stick to the setting of your current environment.</p>"
			"<p>Changes to this setting require a restart of the application to take effect.</p>",
			{ m_screenScaleFactorInfoLabel },
			layout,
			row
		);
		m_screenScaleFactor->addItem("system", -1.0);
		m_screenScaleFactor->addItem("25%", 0.25);
		m_screenScaleFactor->addItem("50%", 0.5);
		m_screenScaleFactor->addItem("75%", 0.75);
		m_screenScaleFactor->addItem("100%", 1.0);
		m_screenScaleFactor->addItem("125%", 1.25);
		m_screenScaleFactor->addItem("150%", 1.5);
		m_screenScaleFactor->addItem("175%", 1.75);
		m_screenScaleFactor->addItem("200%", 2.0);
		m_screenScaleFactor->addItem("250%", 2.5);
		m_screenScaleFactor->addItem("300%", 3.0);
		m_screenScaleFactor->addItem("400%", 4.0);
		connect(m_screenScaleFactor, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated),
			this, &QtProjectWizzardContentPreferences::uiScaleFactorChanges);

		addGap(layout, row);
	}

	// Controls
	addTitle("CONTROLS", layout, row);

	// scroll speed
	m_scrollSpeed = addLineEdit(
		"Scroll Speed",
		"<p>Set a multiplier for the in app scroll speed.</p>"
		"<p>A value between 0 and 1 results in slower scrolling while a value higher than 1 increases scroll speed.</p>",
		layout, row
	);

	// graph zooming
	QString modifierName =utility::getOsType() == OS_MAC ? "Cmd" : "Ctrl";
	m_graphZooming = addCheckBox(
		"Graph Zoom",
		"Zoom graph on mouse wheel",
		"<p>Enable graph zoom using mouse wheel only, instead of using " + modifierName + " + Mouse Wheel.</p>",
		layout, row
	);

	addGap(layout, row);

	// output
	addTitle("OUTPUT", layout, row);

	// logging
	m_loggingEnabled = addCheckBox("Logging", "Enable console and file logging",
		"<p>Show logs in the console and save this information in files.</p>", layout, row);
	connect(m_loggingEnabled, &QCheckBox::clicked, this, &QtProjectWizzardContentPreferences::loggingEnabledChanged);

	m_verboseIndexerLoggingEnabled = addCheckBox(
		"Indexer Logging",
		"Enable verbose indexer logging",
		"<p>Enable additional logs of abstract syntax tree traversal during indexing. This information can help "
		"tracking down crashes that occurr during indexing.</p>"
		"<p><b>Warning</b>: This slows down indexing performance a lot.</p>",
		layout, row
	);

	addGap(layout, row);

	// Network
	addTitle("NETWORK", layout, row);

	// Update check
	m_automaticUpdateCheck = addCheckBox("Automatic<br />Update Check", "Check automatically for updates",
		"<p>Check automatically for a new releases once a day.</p>", layout, row);
	addGap(layout, row);

	// Plugins
	addTitle("PLUGIN", layout, row);

	// Sourcetrail port
	m_sourcetrailPort = addLineEdit("Sourcetrail Port",
		"<p>Port number that Sourcetrail uses to listen for incoming messages from plugins.</p>", layout, row);

	// Sourcetrail port
	m_pluginPort = addLineEdit("Plugin Port",
		"<p>Port number that Sourcetrail uses to sends outgoing messages to plugins.</p>", layout, row);

	addGap(layout, row);

	// indexing
	addTitle("INDEXING", layout, row);

	// indexer threads
	m_threadsInfoLabel = new QLabel("");
	utility::setWidgetRetainsSpaceWhenHidden(m_threadsInfoLabel);
	m_threads = addComboBoxWithWidgets(
		"Indexer Threads",
		0,
		24,
		"<p>Set the number of threads used to work on indexing your project in parallel.</p>"
		"<p>When setting this value to 0 Sourcetrail tries to use the ideal thread count for your computer.</p>",
		{ m_threadsInfoLabel },
		layout,
		row
	);
	connect(m_threads, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated),
		this, &QtProjectWizzardContentPreferences::indexerThreadsChanges);

	// multi process indexing
	m_multiProcessIndexing = addCheckBox("Multi Process<br />C/C++ Indexing",
		"Run C/C++ indexer threads in different process",
		"<p>Enable C/C++ indexer threads to run in different process.</p>"
		"<p>This prevents the application from crashing due to unforseen exceptions while indexing.</p>",
		layout, row);

	addGap(layout, row);


	// Java
	addTitle("JAVA", layout, row);

	{
		// jvm library path
		m_javaPath = new QtLocationPicker(this);

		switch (utility::getOsType())
		{
		case OS_WINDOWS:
			m_javaPath->setFileFilter("JVM Library (jvm.dll)");
			m_javaPath->setPlaceholderText("<jre_path>/bin/client/jvm.dll");
			break;
		case OS_MAC:
			m_javaPath->setFileFilter("JLI or JVM Library (libjli.dylib libjvm.dylib)");
			m_javaPath->setPlaceholderText("<jre_path>/Contents/Home/jre/lib/jli/libjli.dylib");
			break;
		case OS_LINUX:
			m_javaPath->setFileFilter("JVM Library (libjvm.so)");
			m_javaPath->setPlaceholderText("<jre_path>/bin/<arch>/server/libjvm.so");
			break;
		default:
			LOG_WARNING("No placeholders and filters set for Java path selection");
			break;
		}

		const std::string javaArchitectureString =
			utility::getApplicationArchitectureType() == APPLICATION_ARCHITECTURE_X86_32 ? "32 Bit" : "64 Bit";

		addLabelAndWidget(
			("Java Path (" + javaArchitectureString + ")").c_str(),
			m_javaPath,
			layout,
			row
		);

		const std::string javaVersionString = javaArchitectureString + " Java 8";

		addHelpButton(
			"Java Path", (
			"<p>Only required for indexing Java projects.</p>"
			"<p>Provide the location of the jvm library inside the installation of your " + javaVersionString +
			" runtime environment (for information on how to set this take a look at "
			"<a href=\"https://sourcetrail.com/documentation/#FindingJavaRuntimeLibraryLocation\">"
			"Finding Java Runtime Library Location</a> or use the auto detection below)</p>").c_str(),
			layout, row
		);
		row++;

		m_javaPathDetector = utility::getJavaRuntimePathDetector();
		addJavaPathDetection(layout, row);
	}
	{
		// jvm max memory
		m_jvmMaximumMemory = addLineEdit(
			"JVM Maximum Memory",
			"<p>Specify the maximum amount of memory that will be allocated by the indexer's JVM (values are in MB). Set "
			"this value to -1 to use the JVM's default setting.</p>"
			"<p><b>Warning</b>: You may experience a sudden slowdown during the course of indexing when setting this value "
			"too low. This may also happen when using the JVM's default setting.</p>",
			layout, row
		);
		layout->setRowMinimumHeight(row - 1, 30);
	}
	{
		// JRE System Library
		const QString title = "JRE System Library";
		QLabel* label = createFormLabel(title);
		layout->addWidget(label, row, QtProjectWizzardWindow::FRONT_COL, Qt::AlignTop);

		addHelpButton(
			"JRE System Library",
			"<p>Only required for indexing Java projects.</p>"
			"<p>Add the jar files of your JRE System Library. These jars can be found inside your JRE install directory.</p>",
			layout, row);

		m_jreSystemLibraryPaths = new QtDirectoryListBox(this, title);

		layout->addWidget(m_jreSystemLibraryPaths, row, QtProjectWizzardWindow::BACK_COL);
		row++;

		m_jreSystemLibraryPathsDetector = utility::getJreSystemLibraryPathsDetector();
		addJreSystemLibraryPathsDetection(layout, row);
	}
	{
		// maven path
		m_mavenPath = new QtLocationPicker(this);

		if (QSysInfo::windowsVersion() != QSysInfo::WV_None)
		{
			m_mavenPath->setFileFilter("Maven command (mvn.cmd)");
			m_mavenPath->setPlaceholderText("<maven_path>/bin/mvn.cmd");
		}
		else
		{
			m_mavenPath->setFileFilter("Maven command (mvn)");
			m_mavenPath->setPlaceholderText("<binarypath>/mvn");
		}

		addLabelAndWidget("Maven Path", m_mavenPath, layout, row);

		addHelpButton(
			"Maven Path",
			"<p>Only required for indexing projects using Maven.</p>"
			"<p>Provide the location of your installed Maven executable. You can also use the auto detection below.</p>"
			, layout, row
		);
		row++;

		m_mavenPathDetector = utility::getMavenExecutablePathDetector();
		addMavenPathDetection(layout, row);
	}

	addGap(layout, row);

	// C/C++
	addTitle("C/C++", layout, row);
}

void QtProjectWizzardContentPreferences::load()
{
	ApplicationSettings* appSettings = ApplicationSettings::getInstance().get();

	m_fontFace->setCurrentText(QString::fromStdString(appSettings->getFontName()));

	m_fontSize->setCurrentIndex(appSettings->getFontSize() - appSettings->getFontSizeMin());
	m_tabWidth->setCurrentIndex(appSettings->getCodeTabWidth() - 1);

	m_textEncoding->setCurrentText(QString::fromStdString(appSettings->getTextEncoding()));

	FilePath colorSchemePath = appSettings->getColorSchemePath();
	for (size_t i = 0; i < m_colorSchemePaths.size(); i++)
	{
		if (colorSchemePath == m_colorSchemePaths[i])
		{
			m_colorSchemes->setCurrentIndex(i);
			m_oldColorSchemeIndex = i;
			m_newColorSchemeIndex = i;
			break;
		}
	}

	m_useAnimations->setChecked(appSettings->getUseAnimations());
	m_showBuiltinTypes->setChecked(appSettings->getShowBuiltinTypesInGraph());

	if (m_screenAutoScaling)
	{
		m_screenAutoScaling->setCurrentIndex(m_screenAutoScaling->findData(appSettings->getScreenAutoScaling()));
		uiAutoScalingChanges(m_screenAutoScaling->currentIndex());
	}

	if (m_screenScaleFactor)
	{
		m_screenScaleFactor->setCurrentIndex(m_screenScaleFactor->findData(appSettings->getScreenScaleFactor()));
		uiScaleFactorChanges(m_screenScaleFactor->currentIndex());
	}

	m_scrollSpeed->setText(QString::number(appSettings->getScrollSpeed(), 'f', 1));
	m_graphZooming->setChecked(appSettings->getControlsGraphZoomOnMouseWheel());

	m_loggingEnabled->setChecked(appSettings->getLoggingEnabled());
	m_verboseIndexerLoggingEnabled->setChecked(appSettings->getVerboseIndexerLoggingEnabled());
	m_verboseIndexerLoggingEnabled->setEnabled(m_loggingEnabled->isChecked());

	m_automaticUpdateCheck->setChecked(appSettings->getAutomaticUpdateCheck());

	m_sourcetrailPort->setText(QString::number(appSettings->getSourcetrailPort()));
	m_pluginPort->setText(QString::number(appSettings->getPluginPort()));

	m_threads->setCurrentIndex(appSettings->getIndexerThreadCount()); // index and value are the same
	indexerThreadsChanges(m_threads->currentIndex());
	m_multiProcessIndexing->setChecked(appSettings->getMultiProcessIndexingEnabled());

	if (m_javaPath)
	{
		m_javaPath->setText(QString::fromStdString(appSettings->getJavaPath()));
	}

	m_jvmMaximumMemory->setText(QString::number(appSettings->getJavaMaximumMemory()));

	m_jreSystemLibraryPaths->setList(appSettings->getJreSystemLibraryPaths());

	if (m_mavenPath)
	{
		m_mavenPath->setText(QString::fromStdString(appSettings->getMavenPath().str()));
	}
}

void QtProjectWizzardContentPreferences::save()
{
	ApplicationSettings* appSettings = ApplicationSettings::getInstance().get();

	appSettings->setFontName(m_fontFace->currentText().toStdString());

	appSettings->setFontSize(m_fontSize->currentIndex() + appSettings->getFontSizeMin());
	appSettings->setCodeTabWidth(m_tabWidth->currentIndex() + 1);

	appSettings->setTextEncoding(m_textEncoding->currentText().toStdString());

	appSettings->setColorSchemePath(m_colorSchemePaths[m_colorSchemes->currentIndex()]);
	m_oldColorSchemeIndex = -1;

	appSettings->setUseAnimations(m_useAnimations->isChecked());
	appSettings->setShowBuiltinTypesInGraph(m_showBuiltinTypes->isChecked());

	if (m_screenAutoScaling)
	{
		appSettings->setScreenAutoScaling(m_screenAutoScaling->currentData().toInt());
	}

	if (m_screenScaleFactor)
	{
		appSettings->setScreenScaleFactor(m_screenScaleFactor->currentData().toDouble());
	}

	float scrollSpeed = m_scrollSpeed->text().toFloat();
	if (scrollSpeed) appSettings->setScrollSpeed(scrollSpeed);

	appSettings->setControlsGraphZoomOnMouseWheel(m_graphZooming->isChecked());

	appSettings->setLoggingEnabled(m_loggingEnabled->isChecked());
	appSettings->setVerboseIndexerLoggingEnabled(m_verboseIndexerLoggingEnabled->isChecked());

	appSettings->setAutomaticUpdateCheck(m_automaticUpdateCheck->isChecked());

	int sourcetrailPort = m_sourcetrailPort->text().toInt();
	if (sourcetrailPort) appSettings->setSourcetrailPort(sourcetrailPort);

	int pluginPort = m_pluginPort->text().toInt();
	if (pluginPort) appSettings->setPluginPort(pluginPort);

	appSettings->setIndexerThreadCount(m_threads->currentIndex()); // index and value are the same
	appSettings->setMultiProcessIndexingEnabled(m_multiProcessIndexing->isChecked());

	if (m_javaPath)
	{
		appSettings->setJavaPath(FilePath(m_javaPath->getText().toStdString()));
	}

	appSettings->setJreSystemLibraryPaths(m_jreSystemLibraryPaths->getList());

	int jvmMaximumMemory = m_jvmMaximumMemory->text().toInt();
	if (jvmMaximumMemory) appSettings->setJavaMaximumMemory(jvmMaximumMemory);

	if (m_mavenPath)
	{
		appSettings->setMavenPath(FilePath(m_mavenPath->getText().toStdString()));
	}

	appSettings->save();
}

bool QtProjectWizzardContentPreferences::check()
{
	return true;
}

void QtProjectWizzardContentPreferences::colorSchemeChanged(int index)
{
	m_newColorSchemeIndex = index;
	MessageSwitchColorScheme(m_colorSchemePaths[index]).dispatch();
}

void QtProjectWizzardContentPreferences::javaPathDetectionClicked()
{
	std::vector<FilePath> paths = m_javaPathDetector->getPaths(m_javaPathDetectorBox->currentText().toStdString());
	if (!paths.empty())
	{
		m_javaPath->setText(paths.front().str().c_str());
	}
}

void QtProjectWizzardContentPreferences::jreSystemLibraryPathsDetectionClicked()
{
	std::vector<FilePath> paths =
		m_jreSystemLibraryPathsDetector->getPaths(m_jreSystemLibraryPathsDetectorBox->currentText().toStdString());
	std::vector<FilePath> oldPaths = m_jreSystemLibraryPaths->getList();
	m_jreSystemLibraryPaths->setList(utility::unique(utility::concat(oldPaths, paths)));
}

void QtProjectWizzardContentPreferences::mavenPathDetectionClicked()
{
	std::vector<FilePath> paths = m_mavenPathDetector->getPaths(m_mavenPathDetectorBox->currentText().toStdString());
	if (!paths.empty())
	{
		m_mavenPath->setText(paths.front().str().c_str());
	}
}

void QtProjectWizzardContentPreferences::loggingEnabledChanged()
{
	m_verboseIndexerLoggingEnabled->setEnabled(m_loggingEnabled->isChecked());
}

void QtProjectWizzardContentPreferences::indexerThreadsChanges(int index)
{
	if (index == 0)
	{
		m_threadsInfoLabel->setText(
			("detected " + std::to_string(utility::getIdealThreadCount()) + " threads to be ideal.").c_str());
		m_threadsInfoLabel->show();
	}
	else
	{
		m_threadsInfoLabel->hide();
	}
}

void QtProjectWizzardContentPreferences::uiAutoScalingChanges(int index)
{
	if (index == 0)
	{
		QString autoScale(qgetenv("QT_AUTO_SCREEN_SCALE_FACTOR_SOURCETRAIL"));
		if (autoScale == "1")
		{
			autoScale = "on";
		}
		else
		{
			autoScale = "off";
		}

		m_screenAutoScalingInfoLabel->setText("detected: '" + autoScale + "'");
		m_screenAutoScalingInfoLabel->show();
	}
	else
	{
		m_screenAutoScalingInfoLabel->hide();
	}
}

void QtProjectWizzardContentPreferences::uiScaleFactorChanges(int index)
{
	if (index == 0)
	{
		QString scale = "100";
		bool ok;
		double scaleFactor = qgetenv("QT_SCALE_FACTOR_SOURCETRAIL").toDouble(&ok);
		if (ok)
		{
			scale = QString::number(int(scaleFactor * 100));
		}

		m_screenScaleFactorInfoLabel->setText("detected: '" + scale + "%'");
		m_screenScaleFactorInfoLabel->show();
	}
	else
	{
		m_screenScaleFactorInfoLabel->hide();
	}
}

void QtProjectWizzardContentPreferences::addJavaPathDetection(QGridLayout* layout, int& row)
{
	std::vector<std::string> detectorNames = m_javaPathDetector->getWorkingDetectorNames();
	if (detectorNames.empty())
	{
		return;
	}

	QLabel* label = new QLabel("Auto detection from:");

	m_javaPathDetectorBox = new QComboBox();

	for (const std::string& detectorName: detectorNames)
	{
		m_javaPathDetectorBox->addItem(detectorName.c_str());
	}

	QPushButton* button = new QPushButton("detect");
	button->setObjectName("windowButton");
	connect(button, &QPushButton::clicked, this, &QtProjectWizzardContentPreferences::javaPathDetectionClicked);

	QHBoxLayout* hlayout = new QHBoxLayout();
	hlayout->setContentsMargins(0, 0, 0, 0);
	hlayout->addWidget(label);
	hlayout->addWidget(m_javaPathDetectorBox);
	hlayout->addWidget(button);

	QWidget* detectionWidget = new QWidget();
	detectionWidget->setLayout(hlayout);

	layout->addWidget(detectionWidget, row, QtProjectWizzardWindow::BACK_COL, Qt::AlignLeft | Qt::AlignTop);
	row++;
}

void QtProjectWizzardContentPreferences::addJreSystemLibraryPathsDetection(QGridLayout* layout, int& row)
{
	const std::vector<std::string> detectorNames = m_jreSystemLibraryPathsDetector->getWorkingDetectorNames();
	if (detectorNames.empty())
	{
		return;
	}

	QLabel* label = new QLabel("Auto detection from:");

	m_jreSystemLibraryPathsDetectorBox = new QComboBox();

	for (const std::string& detectorName: detectorNames)
	{
		m_jreSystemLibraryPathsDetectorBox->addItem(detectorName.c_str());
	}

	QPushButton* button = new QPushButton("detect");
	button->setObjectName("windowButton");
	connect(button, &QPushButton::clicked, this, &QtProjectWizzardContentPreferences::jreSystemLibraryPathsDetectionClicked);

	QHBoxLayout* hlayout = new QHBoxLayout();
	hlayout->setContentsMargins(0, 0, 0, 0);
	hlayout->addWidget(label);
	hlayout->addWidget(m_jreSystemLibraryPathsDetectorBox);
	hlayout->addWidget(button);

	QWidget* detectionWidget = new QWidget();
	detectionWidget->setLayout(hlayout);

	layout->addWidget(detectionWidget, row, QtProjectWizzardWindow::BACK_COL, Qt::AlignLeft | Qt::AlignTop);
	row++;
}

void QtProjectWizzardContentPreferences::addMavenPathDetection(QGridLayout* layout, int& row)
{
	std::vector<std::string> detectorNames = m_mavenPathDetector->getWorkingDetectorNames();
	if (detectorNames.empty())
	{
		return;
	}

	QLabel* label = new QLabel("Auto detection from:");

	m_mavenPathDetectorBox = new QComboBox();

	for (const std::string& detectorName: detectorNames)
	{
		m_mavenPathDetectorBox->addItem(detectorName.c_str());
	}

	QPushButton* button = new QPushButton("detect");
	button->setObjectName("windowButton");
	connect(button, &QPushButton::clicked, this, &QtProjectWizzardContentPreferences::mavenPathDetectionClicked);

	QHBoxLayout* hlayout = new QHBoxLayout();
	hlayout->setContentsMargins(0, 0, 0, 0);
	hlayout->addWidget(label);
	hlayout->addWidget(m_mavenPathDetectorBox);
	hlayout->addWidget(button);

	QWidget* detectionWidget = new QWidget();
	detectionWidget->setLayout(hlayout);

	layout->addWidget(detectionWidget, row, QtProjectWizzardWindow::BACK_COL, Qt::AlignLeft | Qt::AlignTop);
	row++;
}

void QtProjectWizzardContentPreferences::addTitle(QString title, QGridLayout* layout, int& row)
{
	layout->addWidget(createFormTitle(title), row++, QtProjectWizzardWindow::FRONT_COL, Qt::AlignLeft);
}

void QtProjectWizzardContentPreferences::addLabelAndWidget(
	QString label, QWidget* widget, QGridLayout* layout, int& row, Qt::Alignment widgetAlignment)
{
	layout->addWidget(createFormLabel(label), row, QtProjectWizzardWindow::FRONT_COL, Qt::AlignRight);
	layout->addWidget(widget, row, QtProjectWizzardWindow::BACK_COL, widgetAlignment);
}

void QtProjectWizzardContentPreferences::addGap(QGridLayout* layout, int& row)
{
	layout->setRowMinimumHeight(row++, 20);
}

QCheckBox* QtProjectWizzardContentPreferences::addCheckBox(
	QString label, QString text, QString helpText, QGridLayout* layout, int& row)
{
	QCheckBox* checkBox = new QCheckBox(text, this);
	addLabelAndWidget(label, checkBox, layout, row, Qt::AlignLeft);

	if (helpText.size())
	{
		addHelpButton(label, helpText, layout, row);
	}

	row++;

	return checkBox;
}

QComboBox* QtProjectWizzardContentPreferences::addComboBox(
	QString label, QString helpText, QGridLayout* layout, int& row)
{
	QComboBox* comboBox = new QComboBox(this);
	addLabelAndWidget(label, comboBox, layout, row, Qt::AlignLeft);

	if (helpText.size())
	{
		addHelpButton(label, helpText, layout, row);
	}

	row++;

	return comboBox;
}

QComboBox* QtProjectWizzardContentPreferences::addComboBoxWithWidgets(
	QString label, QString helpText, std::vector<QWidget*> widgets, QGridLayout* layout, int& row)
{
	QComboBox* comboBox = new QComboBox(this);

	QHBoxLayout* hlayout = new QHBoxLayout();
	hlayout->setContentsMargins(0, 0, 0, 0);
	hlayout->addWidget(comboBox);

	for (QWidget* widget : widgets)
	{
		hlayout->addWidget(widget);
	}

	QWidget* container = new QWidget();
	container->setLayout(hlayout);

	addLabelAndWidget(label, container, layout, row, Qt::AlignLeft);

	if (helpText.size())
	{
		addHelpButton(label, helpText, layout, row);
	}

	row++;

	return comboBox;
}

QComboBox* QtProjectWizzardContentPreferences::addComboBox(
	QString label, int min, int max, QString helpText, QGridLayout* layout, int& row)
{
	QComboBox* comboBox = addComboBox(label, helpText, layout, row);

	if (min != max)
	{
		for (int i = min; i <= max; i++)
		{
			comboBox->insertItem(i, QString::number(i));
		}
	}

	return comboBox;
}

QComboBox* QtProjectWizzardContentPreferences::addComboBoxWithWidgets(
	QString label, int min, int max, QString helpText, std::vector<QWidget*> widgets, QGridLayout* layout, int& row)
{
	QComboBox* comboBox = addComboBoxWithWidgets(label, helpText, widgets, layout, row);

	if (min != max)
	{
		for (int i = min; i <= max; i++)
		{
			comboBox->insertItem(i, QString::number(i));
		}
	}

	return comboBox;
}

QLineEdit* QtProjectWizzardContentPreferences::addLineEdit(QString label, QString helpText, QGridLayout* layout, int& row)
{
	QLineEdit* lineEdit = new QLineEdit(this);
	lineEdit->setObjectName("name");
	lineEdit->setAttribute(Qt::WA_MacShowFocusRect, 0);

	addLabelAndWidget(label, lineEdit, layout, row);

	if (helpText.size())
	{
		addHelpButton(label, helpText, layout, row);
	}

	row++;

	return lineEdit;
}
