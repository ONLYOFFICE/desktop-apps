import pdfsIcon from './icons/pdfs.svg';
import docsIcon from './icons/docs.svg';
import sheetsIcon from './icons/sheets.svg';
import othersIcon from './icons/others.svg';

export const tools = [
  {
    id: 'pdf',
    title: 'PDF',
    icon: pdfsIcon,
    cards: [
      {
        title: 'Merge PDF',
        description: 'Merge files into one, with style alignment option',
        prompt: 'Please merge the attached PDF files into a single document. Align styles and formatting for a consistent result across the merged file.',
      },
      {
        title: 'Convert PDF',
        description: 'Convert PDF to Word, Excel, or other formats',
        prompt: 'Please convert the attached PDF to the requested format (Word, Excel, or other). Preserve the original layout, tables, and formatting as accurately as possible.',
      },
      {
        title: 'Split PDF',
        description: 'Split a PDF into separate file',
        prompt: 'Please split the attached PDF into separate files. Let me know how you would like it split — by page, by range, or by section.',
      },
      {
        title: 'Compress PDF',
        description: 'Reduce file size while keeping quality',
        prompt: 'Please compress the attached PDF to reduce its file size while maintaining acceptable visual quality.',
      },
      {
        title: 'Protect PDF',
        description: 'Add password protection to a PDF file',
        prompt: 'Please add password protection to the attached PDF. Specify your desired password, or I will generate a secure one for you.',
      },
      {
        title: 'Extract PDF pages',
        description: 'Extract selected pages into a new PDF',
        prompt: 'Please extract the specified pages from the attached PDF and save them as a new document. Let me know which page numbers or ranges to extract.',
      },
      {
        title: 'Add coments',
        description: 'Add notes, highlights, and annotations to a PDF',
        prompt: 'Please add annotations, highlights, and notes to the attached PDF. Describe what you would like to annotate and where.',
      },
      {
        title: 'Compare PDFs',
        description: 'Compare two PDFs and highlight differences',
        prompt: 'Please compare the two attached PDF files and highlight all differences between them, including text changes, added or removed content, and formatting differences.',
      },
      {
        title: 'Extract images',
        description: 'Extract images from a PDF file',
        prompt: 'Please extract all images from the attached PDF file and provide them as separate image files.',
      },
      {
        title: 'Prepare form',
        description: 'Create fillable fields in a PDF form',
        prompt: 'Please create fillable form fields in the attached PDF document. Identify appropriate places for text fields, checkboxes, dropdowns, and signature fields.',
      },
      {
        title: 'Fill & sign',
        description: 'Fill in form fields and sign a PDF',
        prompt: 'Please fill in the form fields in the attached PDF with the provided information and add a signature where required.',
      },
      {
        title: 'Redact PDF',
        description: 'Permanently remove sensitive information from a PDF',
        prompt: 'Please permanently redact all sensitive information from the attached PDF, such as names, addresses, phone numbers, and other personal or confidential data.',
      },
    ],
  },
  {
    id: 'documents',
    title: 'Documents',
    icon: docsIcon,
    cards: [
      {
        title: 'Compare Documents',
        description: 'Comparing and highlighting differences versions',
        prompt: 'Please compare the two attached documents and highlight all differences between them, including text changes, insertions, deletions, and formatting differences.',
      },
      {
        title: 'Convert to EPUB',
        description: 'Convert your documents into a book',
        prompt: 'Please convert the attached document into EPUB format, preserving the structure, headings, and formatting so it reads well on e-readers.',
      },
      {
        title: 'Clear formating',
        description: 'Clean styles, fonts, colors',
        prompt: 'Please remove all custom formatting from the attached document — including styles, fonts, colors, and spacing — leaving clean, consistently formatted plain text.',
      },
    ],
  },
  {
    id: 'spreadsheets',
    title: 'Spreadsheets',
    icon: sheetsIcon,
    cards: [
      {
        title: 'Remove Duplicates',
        description: 'Remove duplicates for cells',
        prompt: 'Please remove all duplicate rows or entries from the attached spreadsheet and return the cleaned data.',
      },
      {
        title: 'Generate Charts',
        description: 'Generate charts from spreadsheet data',
        prompt: 'Please analyze the data in the attached spreadsheet and generate appropriate charts or visualizations. Suggest the best chart types for the data and explain what each chart shows.',
      },
      {
        title: 'Convert to CSV',
        description: 'Convert your file to CSV format',
        prompt: 'Please convert the attached spreadsheet to CSV format. If it has multiple sheets, let me know which one to convert or export all sheets as separate CSV files.',
      },
      {
        title: 'Convert to ODS',
        description: 'Convert your file to ODS format',
        prompt: 'Please convert the attached spreadsheet to ODS (OpenDocument Spreadsheet) format, preserving all data, formulas, and formatting.',
      },
    ],
  },
  {
    id: 'other',
    title: 'Other Tools',
    icon: othersIcon,
    cards: [
      {
        title: 'Convert Files',
        description: 'Convert files between different formats',
        prompt: 'Please convert the attached file to the requested format. Specify the target format and I will convert it while preserving as much of the original content and structure as possible.',
      },
      {
        title: 'OCR',
        description: 'Extract text from scanned documents and images',
        prompt: 'Please extract all text from the attached scanned document or image using OCR. Return the result as editable text, preserving the original structure and layout where possible.',
      },
    ],
  },
];
