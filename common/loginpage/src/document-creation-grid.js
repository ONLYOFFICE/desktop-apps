"use strict";

/**
 * Document Creation Grid Component
 * Renders a grid of document type cards for creating new documents
 *
 * @param {Object} config - Configuration object
 * @param {Array<DocumentType>} config.documentTypes - Array of document types to display
 * @param {Function} [config.onDocumentSelect] - Callback function when document is selected
 *
 * @typedef {Object} DocumentType
 * @property {string} id - Unique identifier for the document type
 * @property {string} title - Display title for the document
 * @property {string} format - Format label (e.g., 'PDF', 'DOCX')
 * @property {string} icon - CSS class name for the icon
 * @property {string} [color] - Text color for the format label
 * @property {string} [bgColor] - Background color for the card
 *
 * @returns {Object} Component instance with render and control methods
 *
 * @example
 * const docGrid = window.DocumentCreationGrid({
 *   documentTypes: [
 *     {
 *       id: 'pdf',
 *       title: 'PDF Document',
 *       format: 'PDF',
 *       icon: 'icon-pdf',
 *       color: '#E53E3E',
 *       bgColor: '#FED7D7'
 *     }
 *   ],
 *   onDocumentSelect: (docType) => {
 *     console.log('Selected:', docType);
 *   }
 * });
 *
 * docGrid.render(document.getElementById('container'));
 */
window.DocumentCreationGrid = function (config = {}) {
	const {
		documentTypes = [],
		onDocumentSelect = null
	} = config;


	let $el, $parent;
	let currentInstance = null;

	/**
	 * Global handler for document selection
	 * @param {string} docType - Selected document type ID
	 * @param {HTMLElement} element - Clicked element
	 */
	window.DocumentCreationGrid.handleDocumentSelect = function (docType, element) {
		// Find the document type object
		const selectedDoc = documentTypes.find(doc => doc.id === docType);

		if (!selectedDoc) {
			console.warn('Document type not found:', docType);
			return;
		}

		// Emit custom event
		const event = new CustomEvent('documentTypeSelected', {
			detail: {
				type: docType,
				document: selectedDoc,
				element: element
			}
		});
		document.dispatchEvent(event);

		// Call provided callback
		if (onDocumentSelect && typeof onDocumentSelect === 'function') {
			onDocumentSelect(docType, selectedDoc, element);
		}

		console.log('Selected document type:', docType, selectedDoc);
	};

	return {
		/**
		 * Renders the component in the specified parent element
		 * @param {HTMLElement} parentElement - Parent element to render the component in
		 */
		render: (parentElement) => {
			if (!parentElement) {
				throw new Error('Parent element is required for rendering');
			}

			//language=HTML
			const _template = `
                <div class="document-creation-grid">
                    ${documentTypes.map(doc => `
                    <div class="document-creation-item" data-id="${doc.id}" onclick="window.DocumentCreationGrid.handleDocumentSelect('${doc.id}', this)">
                        
                        <div class="format-label" style="--format-bg-start: ${doc.formatLabel.gradientColorStart}; --format-bg-end: ${doc.formatLabel.gradientColorEnd}">
                            <span>${doc.formatLabel.value}</span>
                        </div>
                        
						${doc.icon.startsWith('#') ? `<svg class="icon"><use xlink:href="${doc.icon}"></use></svg>` : `<i class="icon ${doc.icon}"></i>`}
                        
                        <div class="title">
                            ${doc.title}
                        </div>
                    </div>
                `).join('')}
                </div>`;

			$parent = parentElement;
			$el = $parent.append(_template).find('.document-creation-grid');
		},

		/**
		 * Destroys the component and cleans up DOM
		 */
		destroy: () => {
			if (currentInstance && $parent) {
				$parent.removeChild(currentInstance);
			}
			$el = null;
			$parent = null;
			currentInstance = null;
		}
	};
};