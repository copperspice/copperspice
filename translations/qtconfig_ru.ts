<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.0" language="ru">
<context>
    <name>MainWindow</name>
    <message>
        <source>&lt;p&gt;&lt;b&gt;&lt;font size+=2&gt;Appearance&lt;/font&gt;&lt;/b&gt;&lt;/p&gt;&lt;hr&gt;&lt;p&gt;Use this tab to customize the appearance of your Qt applications.&lt;/p&gt;&lt;p&gt;You can select the default GUI Style from the drop down list and customize the colors.&lt;/p&gt;&lt;p&gt;Any GUI Style plugins in your plugin path will automatically be added to the list of built-in Qt styles. (See the Library Paths tab for information on adding new plugin paths.)&lt;/p&gt;&lt;p&gt;When you choose 3-D Effects and Window Background colors, the Qt Configuration program will automatically generate a palette for you. To customize colors further, press the Tune Palette button to open the advanced palette editor.&lt;p&gt;The Preview Window shows what the selected Style and colors look like.</source>
        <translation>&lt;p&gt;&lt;b&gt;&lt;font size+=2&gt;Внешний вид&lt;/font&gt;&lt;/b&gt;&lt;/p&gt;&lt;hr&gt;&lt;p&gt;На этой вкладке можно настроить внешний вид приложений Qt.&lt;/p&gt;&lt;p&gt;Позволяет выбрать стиль интерфейса по умолчанию из выпадающего списка и настроить используемые стилем цвета.&lt;/p&gt;&lt;p&gt;Каждый стиль интерфейса, содержащийся в модулях, найденных в путях к модулям, автоматически добавляется в список встроенных стилей Qt (на вкладке Пути к библиотекам имеется более подробная информация о добавлении путей к модулям).&lt;/p&gt;&lt;p&gt;При выборе эффектов 3D и фоновых цветов окна программа настройки Qt автоматически создаст подходящую палитру. Для дальнейшей настройки цветов следует зайти в расширенный редактор палитры, нажав кнопку Настроить палитру.&lt;p&gt;В окне предпросмотра можно увидеть как будет выглядеть интерфейс с выбранными стилем и цветами.</translation>
    </message>
    <message>
        <source>&lt;p&gt;&lt;b&gt;&lt;font size+=2&gt;Fonts&lt;/font&gt;&lt;/b&gt;&lt;/p&gt;&lt;hr&gt;&lt;p&gt;Use this tab to select the default font for your Qt applications. The selected font is shown (initially as &apos;Sample Text&apos;) in the line edit below the Family, Style and Point Size drop down lists.&lt;/p&gt;&lt;p&gt;Qt has a powerful font substitution feature that allows you to specify a list of substitute fonts.  Substitute fonts are used when a font cannot be loaded, or if the specified font doesn&apos;t have a particular character.&lt;p&gt;For example, if you select the font Lucida, which doesn&apos;t have Korean characters, but need to show some Korean text using the Mincho font family you can do so by adding Mincho to the list. Once Mincho is added, any Korean characters that are not found in the Lucida font will be taken from the Mincho font.  Because the font substitutions are lists, you can also select multiple families, such as Song Ti (for use with Chinese text).</source>
        <translation>&lt;p&gt;&lt;b&gt;&lt;font size+=2&gt;Шрифты&lt;/font&gt;&lt;/b&gt;&lt;/p&gt;&lt;hr&gt;&lt;p&gt;На этой вкладке можно выбрать шрифт по умолчанию для приложений Qt. Выбранный шрифт отображается в строке редактирования ниже выпадающих списков &quot;Шрифт&quot;, &quot;Начертание&quot; и &quot;Размер&quot; (по умолчанию это текст &quot;Текст для примера (Sample Text)&quot;).&lt;/p&gt;&lt;p&gt;Qt обладает мощным механизмом подмены шрифтов, который позволяет задавать список подставляемых шрифтов. Подставляемые шрифты используются, когда шрифт не удаётся загрузить или в нём отсутствуют необходимые символы.&lt;p&gt;Например, если требуется, чтобы при выборе шрифта Lucida, в котором отсутствуют корейские иероглифы, для отображения корейского текста использовался шрифт Mincho,то можно добавить его в список. После этого все корейские символы, отсутствующие в шрифте Lucida, будут браться из шрифта Mincho. Так как для замены используется список, то можно добавлять несколько шрифтов, например, можно также добавить шрифт Song Ti для отображения китайского текста.</translation>
    </message>
    <message>
        <source>&lt;p&gt;&lt;b&gt;&lt;font size+=2&gt;Interface&lt;/font&gt;&lt;/b&gt;&lt;/p&gt;&lt;hr&gt;&lt;p&gt;Use this tab to customize the feel of your Qt applications.&lt;/p&gt;&lt;p&gt;If the Resolve Symlinks checkbox is checked Qt will follow symlinks when handling URLs. For example, in the file dialog, if this setting is turned on and /usr/tmp is a symlink to /var/tmp, entering the /usr/tmp directory will cause the file dialog to change to /var/tmp.  With this setting turned off, symlinks are not resolved or followed.&lt;/p&gt;&lt;p&gt;The Global Strut setting is useful for people who require a minimum size for all widgets (e.g. when using a touch panel or for users who are visually impaired).  Leaving the Global Strut width and height at 0 will disable the Global Strut feature&lt;/p&gt;&lt;p&gt;XIM (Extended Input Methods) are used for entering characters in languages that have large character sets, for example, Chinese and Japanese.</source>
        <translation>&lt;p&gt;&lt;b&gt;&lt;font size+=2&gt;Интерфейс&lt;/font&gt;&lt;/b&gt;&lt;/p&gt;&lt;hr&gt;&lt;p&gt;На этой вкладке можно настроить поведение приложений Qt.&lt;/p&gt;&lt;p&gt;Если включено &quot;Разрешать символьные ссылки&quot;, Qt будет следовать по символьным ссылкам при обработке путей URL. Например, если эта функция включена и /usr/tmp является символьной ссылкой на /var/tmp, то в диалоге выбора файла при вводе пути к каталогу /usr/tmp он будет изменён на /var/tmp.&lt;/p&gt;&lt;p&gt;Функция &quot;Минимальные размеры&quot; предназначены для тех, кому необходимо чтобы элементы интерфейса были не менее заданного размера (например, при использовании сенсорной панели или для людей с проблемами зрения). Если задать 0 в полях &quot;минимальная ширина&quot; и &quot;минимальная высота&quot;, то данная функция будет отключена.&lt;/p&gt;&lt;p&gt;Метод ввода XIM (расширенные методы ввода) используется для ввода символов на языках с большим набором символов (например, китайском или японском).</translation>
    </message>
    <message>
        <source>&lt;p&gt;&lt;b&gt;&lt;font size+=2&gt;Printer&lt;/font&gt;&lt;/b&gt;&lt;/p&gt;&lt;hr&gt;&lt;p&gt;Use this tab to configure the way Qt generates output for the printer.You can specify if Qt should try to embed fonts into its generated output.If you enable font embedding, the resulting postscript will be more portable and will more accurately reflect the visual output on the screen; however the resulting postscript file size will be bigger.&lt;p&gt;When using font embedding you can select additional directories where Qt should search for embeddable font files.  By default, the X server font path is used.</source>
        <translation>&lt;p&gt;&lt;b&gt;&lt;font size+=2&gt;Принтер&lt;/font&gt;&lt;/b&gt;&lt;/p&gt;&lt;hr&gt;&lt;p&gt;На этой вкладке можно настроить способ, которым Qt будет подготавливать данные для печати. Можно указать следует ли встраивать шрифты - в этом случае напечатанные документы будут более похожи на те, что на экране, но при этом увеличится объём данных, передаваемых на печатающее устройство.&lt;p&gt;При использовании встраивания шрифтов можно указать дополнительные каталоги, в которых Qt будет искать файлы шрифтов для встраивания. По умолчанию используется путь к шрифтам X сервера.</translation>
    </message>
    <message>
        <source>&lt;p&gt;&lt;b&gt;&lt;font size+=2&gt;Phonon&lt;/font&gt;&lt;/b&gt;&lt;/p&gt;&lt;hr&gt;&lt;p&gt;Use this tab to configure the Phonon GStreamer multimedia backend. &lt;p&gt;It is reccommended to leave all settings on &quot;Auto&quot; to let Phonon determine your settings automatically.</source>
        <translation>&lt;p&gt;&lt;b&gt;&lt;font size+=2&gt;Phonon&lt;/font&gt;&lt;/b&gt;&lt;/p&gt;&lt;hr&gt;&lt;p&gt;На этой вкладке можно настроить мультимедийную подсистему Phonon GStreamer.&lt;p&gt;Рекомендуется оставить значение &quot;Автоматически&quot; для всех настроек, чтобы Phonon определил параметры самостоятельно.</translation>
    </message>
    <message>
        <source>Desktop Settings (Default)</source>
        <translation>Настройки рабочего стола (по умолчанию)</translation>
    </message>
    <message>
        <source>Choose style and palette based on your desktop settings.</source>
        <translation>Выбор стиля и палитры на основе настроек рабочего стола.</translation>
    </message>
    <message>
        <source>On The Spot</source>
        <translation>В тексте</translation>
    </message>
    <message>
        <source>Unknown</source>
        <translation>Неизвестный</translation>
    </message>
    <message>
        <source>Auto (default)</source>
        <translation>Автоматически (по умолчанию)</translation>
    </message>
    <message>
        <source>Choose audio output automatically.</source>
        <translation>Автоматический выбор звукового выхода.</translation>
    </message>
    <message>
        <source>aRts</source>
        <translation>aRts</translation>
    </message>
    <message>
        <source>Experimental aRts support for GStreamer.</source>
        <translation>Экспериментальная поддержка aRts в GStreamer.</translation>
    </message>
    <message>
        <source>Phonon GStreamer backend not available.</source>
        <translation>Модуль поддержки GStreamer недоступен.</translation>
    </message>
    <message>
        <source>Choose render method automatically</source>
        <translation>Автоматический выбор метода отрисовки</translation>
    </message>
    <message>
        <source>X11</source>
        <translation>X11</translation>
    </message>
    <message>
        <source>Use X11 Overlays</source>
        <translation>Использовать оверлеи X11</translation>
    </message>
    <message>
        <source>OpenGL</source>
        <translation>OpenGL</translation>
    </message>
    <message>
        <source>Use OpenGL if available</source>
        <translation>Использовать OpenGL, если возможно</translation>
    </message>
    <message>
        <source>Software</source>
        <translation>Программный</translation>
    </message>
    <message>
        <source>Use simple software rendering</source>
        <translation>Использовать простую программную отрисовку</translation>
    </message>
    <message>
        <source>No changes to be saved.</source>
        <translation>Нет изменений для сохранения.</translation>
    </message>
    <message>
        <source>Saving changes...</source>
        <translation>Сохранение изменений...</translation>
    </message>
    <message>
        <source>Saved changes.</source>
        <translation>Сохранённые изменения.</translation>
    </message>
    <message>
        <source>Over The Spot</source>
        <translation>Поверх текста</translation>
    </message>
    <message>
        <source>Off The Spot</source>
        <translation>Вне текста</translation>
    </message>
    <message>
        <source>Root</source>
        <translation>Общий</translation>
    </message>
    <message>
        <source>Select a Directory</source>
        <translation>Выбор каталога</translation>
    </message>
    <message>
        <source>&lt;h3&gt;%1&lt;/h3&gt;&lt;br/&gt;Version %2&lt;br/&gt;&lt;br/&gt;Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).</source>
        <translation>&lt;h3&gt;%1&lt;/h3&gt;&lt;br/&gt;Версия %2&lt;br/&gt;&lt;br/&gt;Copyright (C) 2012 Корпорация Nokia и/или её дочерние подразделения.</translation>
    </message>
    <message>
        <source>Qt Configuration</source>
        <translation>Конфигурация Qt</translation>
    </message>
    <message>
        <source>Save Changes</source>
        <translation>Сохранение изменений</translation>
    </message>
    <message>
        <source>Save changes to settings?</source>
        <translation>Сохранить изменения настроек?</translation>
    </message>
    <message>
        <source>Appearance</source>
        <translation>Внешний вид</translation>
    </message>
    <message>
        <source>GUI Style</source>
        <translation>Стиль пользовательского графического интерфейса</translation>
    </message>
    <message>
        <source>Select GUI &amp;Style:</source>
        <translation>&amp;Стиль интерфейса:</translation>
    </message>
    <message>
        <source>Preview</source>
        <translation>Предпросмотр</translation>
    </message>
    <message>
        <source>Select &amp;Palette:</source>
        <translation>Выбор &amp;палитры:</translation>
    </message>
    <message>
        <source>Active Palette</source>
        <translation>Палитра активных элементов</translation>
    </message>
    <message>
        <source>Inactive Palette</source>
        <translation>Палитра неактивных элементов</translation>
    </message>
    <message>
        <source>Disabled Palette</source>
        <translation>Палитра выключенных элементов</translation>
    </message>
    <message>
        <source>Build Palette</source>
        <translation>Палитра</translation>
    </message>
    <message>
        <source>&amp;Button Background:</source>
        <translation>Фон &amp;кнопки:</translation>
    </message>
    <message>
        <source>Window Back&amp;ground:</source>
        <translation>&amp;Фон окна:</translation>
    </message>
    <message>
        <source>&amp;Tune Palette...</source>
        <translation>&amp;Настроить палитру...</translation>
    </message>
    <message>
        <source>Please use the KDE Control Center to set the palette.</source>
        <translation>Используйте Центр управления KDE для настройки цветов.</translation>
    </message>
    <message>
        <source>Fonts</source>
        <translation>Шрифты</translation>
    </message>
    <message>
        <source>Default Font</source>
        <translation>Шрифт по умолчанию</translation>
    </message>
    <message>
        <source>&amp;Style:</source>
        <translation>&amp;Начертание:</translation>
    </message>
    <message>
        <source>&amp;Point Size:</source>
        <translation>&amp;Размер:</translation>
    </message>
    <message>
        <source>F&amp;amily:</source>
        <translation>&amp;Шрифт:</translation>
    </message>
    <message>
        <source>Sample Text</source>
        <translation>Текст для примера (Sample Text)</translation>
    </message>
    <message>
        <source>Font Substitution</source>
        <translation>Подстановка шрифтов</translation>
    </message>
    <message>
        <source>S&amp;elect or Enter a Family:</source>
        <translation>&amp;Выберите шрифт для замены:</translation>
    </message>
    <message>
        <source>Current Substitutions:</source>
        <translation>Текущие замены:</translation>
    </message>
    <message>
        <source>Up</source>
        <translation>Выше</translation>
    </message>
    <message>
        <source>Down</source>
        <translation>Ниже</translation>
    </message>
    <message>
        <source>Remove</source>
        <translation>Удалить</translation>
    </message>
    <message>
        <source>Select s&amp;ubstitute Family:</source>
        <translation>&amp;Заменять на шрифт:</translation>
    </message>
    <message>
        <source>Add</source>
        <translation>Добавить</translation>
    </message>
    <message>
        <source>Interface</source>
        <translation>Интерфейс</translation>
    </message>
    <message>
        <source>Feel Settings</source>
        <translation>Настройки поведения</translation>
    </message>
    <message>
        <source> ms</source>
        <translation> мс</translation>
    </message>
    <message>
        <source>&amp;Double Click Interval:</source>
        <translation>&amp;Интервал двойного щелчка:</translation>
    </message>
    <message>
        <source>No blinking</source>
        <translation>Без мигания</translation>
    </message>
    <message>
        <source>&amp;Cursor Flash Time:</source>
        <translation>&amp;Период мигания курсора:</translation>
    </message>
    <message>
        <source> lines</source>
        <translation> строк</translation>
    </message>
    <message>
        <source>Wheel &amp;Scroll Lines:</source>
        <translation>&amp;Прокручивать строк при повороте колёсика:</translation>
    </message>
    <message>
        <source>Resolve symlinks in URLs</source>
        <translation>Разрешать символьные ссылки в URL-ах</translation>
    </message>
    <message>
        <source>GUI Effects</source>
        <translation>Эффекты интерфейса</translation>
    </message>
    <message>
        <source>&amp;Enable</source>
        <translation>&amp;Включить</translation>
    </message>
    <message>
        <source>Alt+E</source>
        <translation></translation>
    </message>
    <message>
        <source>&amp;Menu Effect:</source>
        <translation>Эффект &amp;меню:</translation>
    </message>
    <message>
        <source>C&amp;omboBox Effect:</source>
        <translation>Эффект &amp;выпадающего списка:</translation>
    </message>
    <message>
        <source>&amp;ToolTip Effect:</source>
        <translation>Эффект &amp;подсказки:</translation>
    </message>
    <message>
        <source>Tool&amp;Box Effect:</source>
        <translation>Эффект панели &amp;инструментов:</translation>
    </message>
    <message>
        <source>Disable</source>
        <translation>Выключен</translation>
    </message>
    <message>
        <source>Animate</source>
        <translation>Анимация</translation>
    </message>
    <message>
        <source>Fade</source>
        <translation>Затухание</translation>
    </message>
    <message>
        <source>Global Strut</source>
        <translation>Минимальные размеры</translation>
    </message>
    <message>
        <source>Minimum &amp;Width:</source>
        <translation>Минимальная &amp;ширина:</translation>
    </message>
    <message>
        <source>Minimum Hei&amp;ght:</source>
        <translation>Минимальная в&amp;ысота:</translation>
    </message>
    <message>
        <source> pixels</source>
        <translation> пикселей</translation>
    </message>
    <message>
        <source>Enhanced support for languages written right-to-left</source>
        <translation>Расширенная поддержка письма справа налево</translation>
    </message>
    <message>
        <source>XIM Input Style:</source>
        <translation>Стиль ввода XIM:</translation>
    </message>
    <message>
        <source>Default Input Method:</source>
        <translation>Метод ввода по умолчанию:</translation>
    </message>
    <message>
        <source>Printer</source>
        <translation>Принтер</translation>
    </message>
    <message>
        <source>Enable Font embedding</source>
        <translation>Разрешить встраивание шрифтов</translation>
    </message>
    <message>
        <source>Font Paths</source>
        <translation>Пути к шрифтам</translation>
    </message>
    <message>
        <source>Browse...</source>
        <translation>Обзор...</translation>
    </message>
    <message>
        <source>Press the &lt;b&gt;Browse&lt;/b&gt; button or enter a directory and press Enter to add them to the list.</source>
        <translation>Нажмите кнопку &lt;b&gt;Обзор...&lt;/b&gt; или укажите каталог и нажмите Ввод для добавления его в список.</translation>
    </message>
    <message>
        <source>Phonon</source>
        <translation>Phonon</translation>
    </message>
    <message>
        <source>About Phonon</source>
        <translation>О Phonon</translation>
    </message>
    <message>
        <source>Current Version:</source>
        <translation>Текущая версия:</translation>
    </message>
    <message>
        <source>Not available</source>
        <translation>Недоступно</translation>
    </message>
    <message>
        <source>Website:</source>
        <translation>Вэб-сайт:</translation>
    </message>
    <message>
        <source>&lt;a href=&quot;http://phonon.kde.org&quot;&gt;http://phonon.kde.org/&lt;/a&gt;</source>
        <translation></translation>
    </message>
    <message>
        <source>About GStreamer</source>
        <translation>О GStreamer</translation>
    </message>
    <message>
        <source>&lt;a href=&quot;http://gstreamer.freedesktop.org/&quot;&gt;http://gstreamer.freedesktop.org/&lt;/a&gt;</source>
        <translation></translation>
    </message>
    <message>
        <source>GStreamer backend settings</source>
        <translation>Настройки модуля GStreamer</translation>
    </message>
    <message>
        <source>Preferred audio sink:</source>
        <translation>Предпочитаемое звуковое устройство:</translation>
    </message>
    <message>
        <source>Preferred render method:</source>
        <translation>Предпочитаемый метод отрисовки:</translation>
    </message>
    <message>
        <source>&lt;!DOCTYPE HTML PUBLIC &quot;-//W3C//DTD HTML 4.0//EN&quot; &quot;http://www.w3.org/TR/REC-html40/strict.dtd&quot;&gt;
&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-style:italic;&quot;&gt;Note: changes to these settings may prevent applications from starting up correctly.&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;!DOCTYPE HTML PUBLIC &quot;-//W3C//DTD HTML 4.0//EN&quot; &quot;http://www.w3.org/TR/REC-html40/strict.dtd&quot;&gt;
&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-style:italic;&quot;&gt;Внимание: Изменение данных настроек может повлечь невозможность корректного запуска приложений.&lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>&amp;File</source>
        <translation>&amp;Файл</translation>
    </message>
    <message>
        <source>&amp;Help</source>
        <translation>&amp;Справка</translation>
    </message>
    <message>
        <source>&amp;Save</source>
        <translation>&amp;Сохранить</translation>
    </message>
    <message>
        <source>Save</source>
        <translation>Сохранить</translation>
    </message>
    <message>
        <source>Ctrl+S</source>
        <translation></translation>
    </message>
    <message>
        <source>E&amp;xit</source>
        <translation>В&amp;ыход</translation>
    </message>
    <message>
        <source>Exit</source>
        <translation>Выход</translation>
    </message>
    <message>
        <source>Ctrl+Q</source>
        <translation></translation>
    </message>
    <message>
        <source>&amp;About</source>
        <translation>&amp;О программе</translation>
    </message>
    <message>
        <source>About</source>
        <translation>О программе</translation>
    </message>
    <message>
        <source>About &amp;Qt</source>
        <translation>О &amp;Qt</translation>
    </message>
    <message>
        <source>About Qt</source>
        <translation>О Qt</translation>
    </message>
</context>
<context>
    <name>PaletteEditorAdvanced</name>
    <message>
        <source>Tune Palette</source>
        <translation>Настройка палитры</translation>
    </message>
    <message>
        <source>Select &amp;Palette:</source>
        <translation>Выбор &amp;палитры:</translation>
    </message>
    <message>
        <source>Active Palette</source>
        <translation>Палитра активных элементов</translation>
    </message>
    <message>
        <source>Inactive Palette</source>
        <translation>Палитра неактивных элементов</translation>
    </message>
    <message>
        <source>Disabled Palette</source>
        <translation>Палитра выключенных элементов</translation>
    </message>
    <message>
        <source>Auto</source>
        <translation>Автоматически</translation>
    </message>
    <message>
        <source>Build inactive palette from active</source>
        <translation>Создать неактивную палитру из активной</translation>
    </message>
    <message>
        <source>Build disabled palette from active</source>
        <translation>Создать выключенную палитру из активной</translation>
    </message>
    <message>
        <source>Central color &amp;roles</source>
        <translation>Основные роли &amp;цветов</translation>
    </message>
    <message>
        <source>Choose central color role</source>
        <translation>Выберите основную роль цвета</translation>
    </message>
    <message>
        <source>&lt;b&gt;Select a color role.&lt;/b&gt;&lt;p&gt;Available central roles are: &lt;ul&gt; &lt;li&gt;Window - general background color.&lt;/li&gt; &lt;li&gt;WindowText - general foreground color. &lt;/li&gt; &lt;li&gt;Base - used as background color for e.g. text entry widgets, usually white or another light color. &lt;/li&gt; &lt;li&gt;Text - the foreground color used with Base. Usually this is the same as WindowText, in what case it must provide good contrast both with Window and Base. &lt;/li&gt; &lt;li&gt;Button - general button background color, where buttons need a background different from Window, as in the Macintosh style. &lt;/li&gt; &lt;li&gt;ButtonText - a foreground color used with the Button color. &lt;/li&gt; &lt;li&gt;Highlight - a color to indicate a selected or highlighted item. &lt;/li&gt; &lt;li&gt;HighlightedText - a text color that contrasts to Highlight. &lt;/li&gt; &lt;li&gt;BrightText - a text color that is very different from WindowText and contrasts well with e.g. black. &lt;/li&gt; &lt;/ul&gt; &lt;/p&gt;</source>
        <translation>&lt;b&gt;Выбор роли цвета.&lt;/b&gt;&lt;p&gt;Доступны следующие роли: &lt;ul&gt;&lt;li&gt;&lt;b&gt;Окно&lt;/b&gt; - основной цвет фона.&lt;/li&gt; &lt;li&gt;&lt;b&gt;Текст окна&lt;/b&gt; - основной цвет текста.&lt;/li&gt; &lt;li&gt;&lt;b&gt;Фон&lt;/b&gt; - используется в качестве фона для, например, виджетов с текстовыми полями, обычно, белый или другой светлый цвет.&lt;/li&gt; &lt;li&gt;&lt;b&gt;Текст&lt;/b&gt; - цвет текста используемый совместно с &lt;b&gt;Фон&lt;/b&gt;. Обычно, он совпадает с &lt;b&gt;Текст окна&lt;/b&gt;, так как в этом случае получается максимальный контраст и с &lt;b&gt;Окно&lt;/b&gt;, и с &lt;b&gt;Фон&lt;/b&gt;.&lt;/li&gt; &lt;li&gt;&lt;b&gt;Кнопка&lt;/b&gt; - основной цвет фона кнопки, которой требуется цвет отличный от &lt;b&gt;Окно&lt;/b&gt;, например, в стиле Macintosh.&lt;/li&gt; &lt;li&gt;&lt;b&gt;Текст кнопки&lt;/b&gt; - цвет текста используемый совместно с &lt;b&gt;Кнопка&lt;/b&gt;.&lt;/li&gt; &lt;li&gt;&lt;b&gt;Выделение&lt;/b&gt; - цвет для обозначения выбранного или выделенного элемента.&lt;/li&gt; &lt;li&gt;&lt;b&gt;Выделенный текст&lt;/b&gt; - цвет текста контрастирующий с &lt;b&gt;Выделение&lt;/b&gt;.&lt;/li&gt; &lt;li&gt;&lt;b&gt;Яркий текст&lt;/b&gt; - цвет текста, который отличается от &lt;b&gt;Текст окна&lt;/b&gt; и хорошо контрастирует с черным.&lt;/li&gt;&lt;/ul&gt;&lt;/p&gt;</translation>
    </message>
    <message>
        <source>Window</source>
        <translation>Окно</translation>
    </message>
    <message>
        <source>WindowText</source>
        <translation>Текст окна</translation>
    </message>
    <message>
        <source>Base</source>
        <translation>Фон</translation>
    </message>
    <message>
        <source>AlternateBase</source>
        <translation>Альтернативный Фон
</translation>
    </message>
    <message>
        <source>ToolTipBase</source>
        <translation>Фон подсказки</translation>
    </message>
    <message>
        <source>ToolTipText</source>
        <translation>Текст подсказки</translation>
    </message>
    <message>
        <source>Text</source>
        <translation>Текст</translation>
    </message>
    <message>
        <source>Button</source>
        <translation>Кнопка</translation>
    </message>
    <message>
        <source>ButtonText</source>
        <translation>Текст на кнопке</translation>
    </message>
    <message>
        <source>BrightText</source>
        <translation>Светлый текст</translation>
    </message>
    <message>
        <source>Highlight</source>
        <translation>Выделение</translation>
    </message>
    <message>
        <source>HighlightedText</source>
        <translation>Выделенный текст</translation>
    </message>
    <message>
        <source>Link</source>
        <translation>Ссылка</translation>
    </message>
    <message>
        <source>LinkVisited</source>
        <translation>Посещённая ссылка</translation>
    </message>
    <message>
        <source>&amp;Select Color:</source>
        <translation>&amp;Выбор цвета:</translation>
    </message>
    <message>
        <source>3-D shadow &amp;effects</source>
        <translation>Эффекты т&amp;рехмерной тени</translation>
    </message>
    <message>
        <source>Generate shadings</source>
        <translation>Создание полутонов</translation>
    </message>
    <message>
        <source>Check to let 3D-effect colors be calculated from button-color.</source>
        <translation>Создать цвета эффекта трёхмерности из цвета кнопки.</translation>
    </message>
    <message>
        <source>Build &amp;from button color</source>
        <translation>Получ&amp;ить из цвета кнопки</translation>
    </message>
    <message>
        <source>Choose 3D-effect color role</source>
        <translation>Выбор роли цвета дял эффекта трёхмерности</translation>
    </message>
    <message>
        <source>&lt;b&gt;Select a color role.&lt;/b&gt;&lt;p&gt;Available effect roles are: &lt;ul&gt; &lt;li&gt;Light - lighter than Button color. &lt;/li&gt; &lt;li&gt;Midlight - between Button and Light. &lt;/li&gt; &lt;li&gt;Mid - between Button and Dark. &lt;/li&gt; &lt;li&gt;Dark - darker than Button. &lt;/li&gt; &lt;li&gt;Shadow - a very dark color. &lt;/li&gt; &lt;/ul&gt;</source>
        <translation>&lt;b&gt;Выбор роли цвета.&lt;/b&gt;&lt;p&gt;Доступны следующие роли: &lt;ul&gt; &lt;li&gt;&lt;b&gt;Светлый&lt;/b&gt; - светлее цвета &lt;b&gt;Кнопка&lt;/b&gt;. &lt;/li&gt; &lt;li&gt;&lt;b&gt;Полусветлый&lt;/b&gt; - среднее между &lt;b&gt;Светлый&lt;/b&gt; и &lt;b&gt;Кнопка&lt;/b&gt;. &lt;/li&gt; &lt;li&gt;&lt;b&gt;Полутёмный&lt;/b&gt; - среднее между &lt;b&gt;Кнопка&lt;/b&gt; и &lt;b&gt;Тёмный&lt;/b&gt;. &lt;/li&gt; &lt;li&gt;&lt;b&gt;Тёмный&lt;/b&gt; - темнее цвета &lt;b&gt;Кнопка&lt;/b&gt;. &lt;/li&gt; &lt;li&gt;&lt;b&gt;Тень&lt;/b&gt; - очень темный цвет. &lt;/li&gt; &lt;/ul&gt;</translation>
    </message>
    <message>
        <source>Light</source>
        <translation>Светлый</translation>
    </message>
    <message>
        <source>Midlight</source>
        <translation>Полусветлый</translation>
    </message>
    <message>
        <source>Mid</source>
        <translation>Полутёмный</translation>
    </message>
    <message>
        <source>Dark</source>
        <translation>Тёмный</translation>
    </message>
    <message>
        <source>Shadow</source>
        <translation>Тень</translation>
    </message>
    <message>
        <source>Select Co&amp;lor:</source>
        <translation>Выбор &amp;цвета:</translation>
    </message>
    <message>
        <source>Choose a color</source>
        <translation>Выберите цвет</translation>
    </message>
    <message>
        <source>Choose a color for the selected central color role.</source>
        <translation>Выберите цвет для указанной роли.</translation>
    </message>
    <message>
        <source>Choose a color for the selected effect color role.</source>
        <translation>Выбор цвета для указанной роли.</translation>
    </message>
</context>
<context>
    <name>PreviewFrame</name>
    <message>
        <source>Desktop settings will only take effect after an application restart.</source>
        <translation>Настройки рабочего стола применятся после перезапуска приложения.</translation>
    </message>
</context>
<context>
    <name>PreviewWidget</name>
    <message>
        <source>Preview Window</source>
        <translation>Окно предпросмотра</translation>
    </message>
    <message>
        <source>GroupBox</source>
        <translation type="unfinished">Объединение</translation>
    </message>
    <message>
        <source>RadioButton1</source>
        <translation type="unfinished">Переключатель1</translation>
    </message>
    <message>
        <source>RadioButton2</source>
        <translation type="unfinished">Переключатель2</translation>
    </message>
    <message>
        <source>RadioButton3</source>
        <translation type="unfinished">Переключатель3</translation>
    </message>
    <message>
        <source>GroupBox2</source>
        <translation type="unfinished">Объединение2</translation>
    </message>
    <message>
        <source>CheckBox1</source>
        <translation type="unfinished">Выключатель1</translation>
    </message>
    <message>
        <source>CheckBox2</source>
        <translation type="unfinished">Выключатель2</translation>
    </message>
    <message>
        <source>LineEdit</source>
        <translation type="unfinished">Строка редактирования</translation>
    </message>
    <message>
        <source>ComboBox</source>
        <translation type="unfinished">Выпадающий список</translation>
    </message>
    <message>
        <source>PushButton</source>
        <translation type="unfinished">Простая кнопка</translation>
    </message>
    <message>
        <source>&lt;p&gt;&lt;a href=&quot;http://qt.nokia.com&quot;&gt;http://qt.nokia.com&lt;/a&gt;&lt;/p&gt;
&lt;p&gt;&lt;a href=&quot;http://www.kde.org&quot;&gt;http://www.kde.org&lt;/a&gt;&lt;/p&gt;</source>
        <translation></translation>
    </message>
</context>
</TS>
