import pdfsIcon from './icons/pdfs.svg';
import docsIcon from './icons/docs.svg';
import sheetsIcon from './icons/sheets.svg';
import othersIcon from './icons/others.svg';
import slideIcon from './icons/slide.svg';

export const groups = [
  { id: 'pdf',   icon: pdfsIcon,   title: 'PDF' },
  { id: 'word',  icon: docsIcon,   title: 'Documents' },
  { id: 'cell',  icon: sheetsIcon, title: 'Spreadsheets' },
  { id: 'other', icon: othersIcon, title: 'Other Tools' },
  { id: 'slide', icon: slideIcon,  title: 'Presentations' },
];

export const iconRegistry = {
  'pdfs.svg':   pdfsIcon,
  'docs.svg':   docsIcon,
  'sheets.svg': sheetsIcon,
  'others.svg': othersIcon,
  'slide.svg':  slideIcon,
};
