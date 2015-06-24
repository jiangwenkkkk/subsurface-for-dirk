#include "printdialog.h"
#include "printoptions.h"
#include "mainwindow.h"

#ifndef NO_PRINTING
#include <QProgressBar>
#include <QPrintPreviewDialog>
#include <QPrintDialog>
#include <QShortcut>
#include <QSettings>
#include <QMessageBox>

#define SETTINGS_GROUP "PrintDialog"

PrintDialog::PrintDialog(QWidget *parent, Qt::WindowFlags f) : QDialog(parent, f)
{
	// check if the options were previously stored in the settings; if not use some defaults.
	QSettings s;
	bool stored = s.childGroups().contains(SETTINGS_GROUP);
	if (!stored) {
		printOptions.print_selected = true;
		printOptions.color_selected = true;
		printOptions.landscape = false;
		printOptions.p_template = print_options::ONE_DIVE;
		printOptions.type = print_options::DIVELIST;
	} else {
		s.beginGroup(SETTINGS_GROUP);
		printOptions.type = (print_options::print_type)s.value("type").toInt();
		printOptions.print_selected = s.value("print_selected").toBool();
		printOptions.color_selected = s.value("color_selected").toBool();
		printOptions.landscape = s.value("landscape").toBool();
		printOptions.p_template = (print_options::print_template)s.value("template_selected").toInt();
		qprinter.setOrientation((QPrinter::Orientation)printOptions.landscape);
	}

	// create a print options object and pass our options struct
	optionsWidget = new PrintOptions(this, &printOptions);

	// create a new printer object
	printer = new Printer(&qprinter, &printOptions);

	QVBoxLayout *layout = new QVBoxLayout(this);
	setLayout(layout);

	layout->addWidget(optionsWidget);

	progressBar = new QProgressBar();
	progressBar->setMinimum(0);
	progressBar->setMaximum(100);
	progressBar->setValue(0);
	progressBar->setTextVisible(false);
	layout->addWidget(progressBar);

	QHBoxLayout *hLayout = new QHBoxLayout();
	layout->addLayout(hLayout);

	QPushButton *printButton = new QPushButton(tr("P&rint"));
	connect(printButton, SIGNAL(clicked(bool)), this, SLOT(printClicked()));

	QPushButton *previewButton = new QPushButton(tr("&Preview"));
	connect(previewButton, SIGNAL(clicked(bool)), this, SLOT(previewClicked()));

	QDialogButtonBox *buttonBox = new QDialogButtonBox;
	buttonBox->addButton(QDialogButtonBox::Cancel);
	buttonBox->addButton(printButton, QDialogButtonBox::AcceptRole);
	buttonBox->addButton(previewButton, QDialogButtonBox::ActionRole);

	connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

	hLayout->addWidget(buttonBox);

	setWindowTitle(tr("Print"));
	setWindowIcon(QIcon(":subsurface-icon"));

	QShortcut *close = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_W), this);
	connect(close, SIGNAL(activated()), this, SLOT(close()));
	QShortcut *quit = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q), this);
	connect(quit, SIGNAL(activated()), parent, SLOT(close()));

	// seems to be the most reliable way to track for all sorts of dialog disposal.
	connect(this, SIGNAL(finished(int)), this, SLOT(onFinished()));
}

void PrintDialog::onFinished()
{
	// save the settings
	QSettings s;
	s.beginGroup(SETTINGS_GROUP);
	s.setValue("type", printOptions.type);
	s.setValue("print_selected", printOptions.print_selected);
	s.setValue("color_selected", printOptions.color_selected);
	s.setValue("template_selected", printOptions.p_template);
}

void PrintDialog::previewClicked(void)
{
	if (printOptions.type == print_options::TABLE || printOptions.type == print_options::STATISTICS) {
		QMessageBox msgBox;
		msgBox.setText("This feature is not implemented yet");
		msgBox.exec();
		return;
	}

	QPrintPreviewDialog previewDialog(&qprinter, this, Qt::Window
		| Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint
		| Qt::WindowTitleHint);
	connect(&previewDialog, SIGNAL(paintRequested(QPrinter *)), this, SLOT(onPaintRequested(QPrinter *)));
	previewDialog.exec();
}

void PrintDialog::printClicked(void)
{
	if (printOptions.type == print_options::TABLE || printOptions.type == print_options::STATISTICS) {
		QMessageBox msgBox;
		msgBox.setText("This feature is not implemented yet");
		msgBox.exec();
		return;
	}

	QPrintDialog printDialog(&qprinter, this);
	if (printDialog.exec() == QDialog::Accepted) {
		switch (printOptions.type) {
		case print_options::DIVELIST:
			connect(printer, SIGNAL(progessUpdated(int)), progressBar, SLOT(setValue(int)));
			printer->print();
			break;
		case print_options::TABLE:
			break;
		case print_options::STATISTICS:
			break;
		}
		close();
	}
}

void PrintDialog::onPaintRequested(QPrinter *printerPtr)
{
	connect(printer, SIGNAL(progessUpdated(int)), progressBar, SLOT(setValue(int)));
	printer->print();
	progressBar->setValue(0);
	disconnect(printer, SIGNAL(progessUpdated(int)), progressBar, SLOT(setValue(int)));
}
#endif
